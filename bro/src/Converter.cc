
#include "Obj.h"
#include "Type.h"
#include "Plugin.h"
#undef List

#include <hilti/builder/builder.h>
#include <binpac/type.h>

#include "Converter.h"
#include "RuntimeInterface.h"
#include "LocalReporter.h"

using namespace bro::hilti;

TypeConverter::TypeConverter()
	{
	}

BroType* TypeConverter::Convert(std::shared_ptr<::hilti::Type> type, std::shared_ptr<::binpac::Type> btype)
	{
	setArg1(btype);
	BroType* rtype;
	processOne(type, &rtype);
	return rtype;
	}

string TypeConverter::CacheIndex(std::shared_ptr<::hilti::Type> type, std::shared_ptr<::binpac::Type> btype)
	{
	return ::util::fmt("%p-%p", type, type);
        }

BroType* TypeConverter::LookupCachedType(std::shared_ptr<::hilti::Type> type, std::shared_ptr<::binpac::Type> btype)
	{
        auto i = type_cache.find(CacheIndex(type, btype));
	return i != type_cache.end() ? i->second.first : nullptr;
        }

uint64_t TypeConverter::TypeIndex(std::shared_ptr<::hilti::Type> type, std::shared_ptr<::binpac::Type> btype)
	{
	Convert(type, btype); // Make sure we have the type cached.
	auto i = type_cache.find(CacheIndex(type, btype));
	assert(i != type_cache.end());
	return i->second.second;
        }

void TypeConverter::CacheType(std::shared_ptr<::hilti::Type> type, std::shared_ptr<::binpac::Type> btype, BroType* bro_type)
	{
	auto cache_idx = CacheIndex(type, btype);
	auto type_idx = lib_bro_add_indexed_type(bro_type);
	type_cache.insert(std::make_pair(cache_idx, std::make_pair(bro_type, type_idx)));
	}

ValueConverter::ValueConverter(shared_ptr<::hilti::builder::ModuleBuilder> arg_mbuilder,
			       shared_ptr<TypeConverter> arg_type_converter)
	{
	mbuilder = arg_mbuilder;
	type_converter = arg_type_converter;
	}

bool ValueConverter::Convert(shared_ptr<::hilti::Expression> value, shared_ptr<::hilti::Expression> dst, std::shared_ptr<::binpac::Type> btype)
	{
	setArg1(value);
	setArg2(dst);
	_arg3 = btype;
	bool set = false;
	bool success = processOne(value->type(), &set);
	assert(set);
	_arg3 = nullptr;
	return success;
	}

shared_ptr<::hilti::builder::BlockBuilder> ValueConverter::Builder() const
	{
	return mbuilder->builder();
	}

shared_ptr<::binpac::Type> ValueConverter::arg3() const
	{
	return _arg3;
	}

void TypeConverter::visit(::hilti::type::Reference* b)
	{
	BroType* rtype;
	processOne(b->argType(), &rtype);
	setResult(rtype);
	}

void TypeConverter::visit(::hilti::type::Address* a)
	{
	auto result = base_type(TYPE_ADDR);
	setResult(result);
	}

void TypeConverter::visit(::hilti::type::Bool* a)
	{
	auto result = base_type(TYPE_BOOL);
	setResult(result);
	}

void TypeConverter::visit(::hilti::type::Bytes* b)
	{
	auto result = base_type(TYPE_STRING);
	setResult(result);
	}

void TypeConverter::visit(::hilti::type::Double* d)
	{
	auto result = base_type(TYPE_DOUBLE);
	setResult(result);
	}

void TypeConverter::visit(::hilti::type::Enum* e)
	{
	auto etype = ast::checkedCast<binpac::type::Enum>(arg1());

	if ( auto result = LookupCachedType(e->sharedPtr<::hilti::type::Enum>(), etype) )
		{
		setResult(result);
		return;
		}

        if ( ! etype->id() )
                reporter::internal_error("TypeConverter::visit(Enum): type has not ID");

        string module;
        string name;
        auto c = etype->id()->path();

        if ( c.size() == 1 )
		{
		name = c.front();

		if ( auto m = etype->id()->firstParent<binpac::Module>() )
			module = m->id()->pathAsString();
		}

	else if ( c.size() == 2 )
		{
		module = c.front();
		name = c.back();
		}

	if ( module.empty() || name.empty() )
                reporter::internal_error(::util::fmt("TypeConverter::visit(Enum): cannot determine type's module/name (%s/%s|%s)",
						     module.size(), name.size(), etype->id()->pathAsString().c_str()));

	// Build the Bro type.
	auto eresult = new EnumType(name);
	eresult->AddName(module, "Undef", lib_bro_enum_undef_val, true);

	for ( auto l : etype->labels() )
		{
		auto name = l.first->name();
		if ( name != "Undef" )
			eresult->AddName(module, name.c_str(), l.second, true);
		}

	PLUGIN_DBG_LOG(HiltiPlugin, "Adding Bro enum type %s::%s", module.c_str(), name.c_str());
	string id_name = ::util::fmt("%s::%s", module, name);
	::ID* id = install_ID(name.c_str(), module.c_str(), true, true);
	id->SetType(eresult);
	id->MakeType();

	CacheType(e->sharedPtr<::hilti::type::Enum>(), etype, eresult);
	setResult(eresult);
	}

void TypeConverter::visit(::hilti::type::Integer* i)
	{
	auto itype = ast::checkedCast<binpac::type::Integer>(arg1());

	auto result = itype->signed_() ? base_type(TYPE_INT) : base_type(TYPE_COUNT);
	setResult(result);
	}

void TypeConverter::visit(::hilti::type::String* s)
	{
	auto result = base_type(TYPE_STRING);
	setResult(result);
	}

void TypeConverter::visit(::hilti::type::Time* t)
	{
	auto result = base_type(TYPE_TIME);
	setResult(result);
        }

void ValueConverter::visit(::hilti::type::Reference* b)
	{
	bool set = false;
	bool success = processOne(b->argType(), &set);
	assert(set);
	setResult(true);
	}

void ValueConverter::visit(::hilti::type::Integer* i)
	{
	auto val = arg1();
	auto dst = arg2();
	auto itype = ast::checkedCast<binpac::type::Integer>(arg3());

	const char* func = "";
	shared_ptr<::hilti::Instruction> ext = 0;

	if ( itype->signed_() )
		{
		func = "LibBro::h2b_integer_signed";
		ext = ::hilti::instruction::integer::SExt;
		}

	else
		{
		func = "LibBro::h2b_integer_unsigned";
		ext = ::hilti::instruction::integer::ZExt;
		}

	if ( itype->width() != 64 )
		{
		auto tmp = Builder()->addTmp("ext", ::hilti::builder::integer::type(64));
		Builder()->addInstruction(tmp, ext, val);
		val = tmp;
		}

	auto args = ::hilti::builder::tuple::create( { val } );
	Builder()->addInstruction(dst, ::hilti::instruction::flow::CallResult,
				  ::hilti::builder::id::create(func), args);
	setResult(true);
	}

void ValueConverter::visit(::hilti::type::Address* a)
	{
	auto val = arg1();
	auto dst = arg2();

	auto args = ::hilti::builder::tuple::create( { val } );
	Builder()->addInstruction(dst, ::hilti::instruction::flow::CallResult,
				  ::hilti::builder::id::create("LibBro::h2b_address"), args);
	setResult(true);
	}

void ValueConverter::visit(::hilti::type::Bool* b)
	{
	auto val = arg1();
	auto dst = arg2();

	auto args = ::hilti::builder::tuple::create( { val } );
	Builder()->addInstruction(dst, ::hilti::instruction::flow::CallResult,
				  ::hilti::builder::id::create("LibBro::h2b_bool"), args);
	setResult(true);
	}

void ValueConverter::visit(::hilti::type::Bytes* b)
	{
	auto val = arg1();
	auto dst = arg2();
	auto btype = arg3();

	auto args = ::hilti::builder::tuple::create( { val } );
	Builder()->addInstruction(dst, ::hilti::instruction::flow::CallResult,
				  ::hilti::builder::id::create("LibBro::h2b_bytes"), args);
	setResult(true);
	}

void ValueConverter::visit(::hilti::type::Double* d)
	{
	auto val = arg1();
	auto dst = arg2();

	auto args = ::hilti::builder::tuple::create( { val } );
	Builder()->addInstruction(dst, ::hilti::instruction::flow::CallResult,
				  ::hilti::builder::id::create("LibBro::h2b_double"), args);
	setResult(true);
	}


void ValueConverter::visit(::hilti::type::Enum* e)
	{
	auto val = arg1();
	auto dst = arg2();
	auto btype = arg3();

	auto type_idx = type_converter->TypeIndex(val->type(), btype);
        auto type_idx_hlt = ::hilti::builder::integer::create(type_idx);
        auto args = ::hilti::builder::tuple::create( { val, type_idx_hlt } );
	Builder()->addInstruction(dst, ::hilti::instruction::flow::CallResult,
				  ::hilti::builder::id::create("LibBro::h2b_enum"), args);
	setResult(true);
        }

void ValueConverter::visit(::hilti::type::String* s)
	{
	auto val = arg1();
	auto dst = arg2();

	auto args = ::hilti::builder::tuple::create( { val } );
	Builder()->addInstruction(dst, ::hilti::instruction::flow::CallResult,
				  ::hilti::builder::id::create("LibBro::h2b_string"), args);
	setResult(true);
	}

void ValueConverter::visit(::hilti::type::Time* t)
	{
	auto val = arg1();
	auto dst = arg2();

	auto args = ::hilti::builder::tuple::create( { val } );
	Builder()->addInstruction(dst, ::hilti::instruction::flow::CallResult,
				  ::hilti::builder::id::create("LibBro::h2b_time"), args);
	setResult(true);
        }