
#ifndef HILTI_VISITOR_INTERFACE_H
#define HILTI_VISITOR_INTERFACE_H

namespace hilti {

class Constant;
class Declaration;
class Expression;
class Function;
class ID;
class Instruction;
class Module;
class Statement;
class Type;
class Variable;

namespace type {
   class Any;
   class Unset;
   class Unknown;
   class Void;
   class String;
   class Integer;
   class Bool;
   class Function;
   class Tuple;
   class Block;
   class Module;
   class Type;
   class Function;

   namespace trait {
      namespace parameter {
          class Base;
          class Type;
          class Integer;
      }
   }

   namespace function {
      class Parameter;
   }
}

namespace constant {
   class Unset;
   class String;
   class Integer;
   class Bool;
   class Tuple;
}

namespace expression {
   class List;
   class Constant;
   class Ctor;
   class Name;
   class Type;
   class Constant;
   class ID;
   class Function;
   class Parameter;
   class Variable;
   class Module;
   class Block;
   class Coerced;
   class CodeGen;
}

namespace declaration {
   class Constant;
   class Function;
   class Variable;
   class Type;
}

namespace variable {
   class Global;
   class Local;
}

namespace statement {
   class Noop;
   class Block;
   class Instruction;

   namespace instruction {
       class Resolved;
       class Unresolved;

       namespace string {
           class Cat;
       }

       namespace integer {
           class Add;
           class Sub;
       }

       namespace flow {
          class ReturnResult;
          class ReturnVoid;
          class CallResult;
          class CallVoid;
       }
   }
}

namespace instruction {
   namespace internal {
       class UnknownInstruction;
       class AmbigiousInstruction;
   }
}

}

#include "common.h"

namespace hilti {

/// Visitor interface. This class defines one visit method for each type of
/// HILTI AST node that we have. Visitor implementation then override the
/// ones they need.
class VisitorInterface
{
public:
   virtual ~VisitorInterface();

   virtual void visit(ID* s)                {}
   virtual void visit(Module* m)            {}

   // Functions.
   virtual void visit(Function* f)          {}
   virtual void visit(type::function::Parameter* p) {}

   // Types.
   virtual void visit(Type* t)              {}
   virtual void visit(type::Any* a)         {}
   virtual void visit(type::Unset* t)       {}
   virtual void visit(type::Void* v)        {}
   virtual void visit(type::String* s)      {}
   virtual void visit(type::Integer* i)     {}
   virtual void visit(type::Bool* b)        {}
   virtual void visit(type::Function* i)    {}
   virtual void visit(type::Tuple* t)       {}

   virtual void visit(type::trait::parameter::Base* b) {}
   virtual void visit(type::trait::parameter::Type* t) {}
   virtual void visit(type::trait::parameter::Integer* i) {}

   // Constants.
   virtual void visit(Constant* s)          {}
   virtual void visit(constant::Unset* u)   {};
   virtual void visit(constant::Integer* i) {}
   virtual void visit(constant::String* i)  {}
   virtual void visit(constant::Bool* i)    {}
   virtual void visit(constant::Tuple* t)   {};

   // Expressions.
   virtual void visit(Expression* e)        {}
   virtual void visit(expression::List* l)  {}
   virtual void visit(expression::Ctor* c)  {}
   virtual void visit(expression::ID* p)    {}
   virtual void visit(expression::Constant* c) {}
   virtual void visit(expression::Variable* v) {}
   virtual void visit(expression::Parameter* p) {}
   virtual void visit(expression::Function* f) {}
   virtual void visit(expression::Module* m) {}
   virtual void visit(expression::Type* t) {}
   virtual void visit(expression::Block* b) {}
   virtual void visit(expression::Coerced* e) {}
   virtual void visit(expression::CodeGen* c) {}

   // Declarations.
   virtual void visit(Declaration* d) {}
   virtual void visit(declaration::Constant* c) {}
   virtual void visit(declaration::Function* f) {}
   virtual void visit(declaration::Variable* v) {}
   virtual void visit(declaration::Type* t) {}

   // Variables
   virtual void visit(Variable* v)          {}
   virtual void visit(variable::Global* g)  {}
   virtual void visit(variable::Local* l)   {}

   // Statements.
   virtual void visit(Statement* s)         {}
   virtual void visit(statement::Noop* s)   {}
   virtual void visit(statement::Block* s)  {}
   virtual void visit(statement::Instruction* s) {}
   virtual void visit(statement::instruction::Resolved* s) {}
   virtual void visit(statement::instruction::Unresolved* s) {}

   virtual void visit(statement::instruction::string::Cat *i) {}
   virtual void visit(statement::instruction::integer::Add *i) {}
   virtual void visit(statement::instruction::integer::Sub *i) {}

   virtual void visit(statement::instruction::flow::ReturnResult* i) {}
   virtual void visit(statement::instruction::flow::ReturnVoid* i) {}
   virtual void visit(statement::instruction::flow::CallResult* i) {}
   virtual void visit(statement::instruction::flow::CallVoid* i) {}

   // Instructions.
   virtual void visit(Instruction* s)       {}

protected:
   /// Internal function. This carries out the visitor callback.
   virtual void callAccept(shared_ptr<ast::NodeBase> node);
};

}

#endif
