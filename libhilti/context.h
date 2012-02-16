///
/// All thread-global state is stored in execution contexts.

#ifndef HLT_CONTEXT_H
#define HLT_CONTEXT_H

#include "memory.h"
#include "types.h"
#include "rtti.h"

#if 0
#include "continuation.h"
#include "profiler.h"
#endif

__HLT_DECLARE_RTTI_GC_TYPE(hlt_execution_context)

// This is generated by the HILTI linker. Calling it initializes all the
// globals in the passed context.
extern void __hlt_module_globals_init(hlt_execution_context* ctx);

// This is generated by the HILTI linker. Calling it cleans up all the
// globals in the passed context.
extern void __hlt_module_globals_dtor(hlt_execution_context* ctx);

struct hlt_worker_thread;

/// A per-thread execution context. This is however just the common header of
/// all contexts. In memory, the header will be followed with the set of
/// thread-global variables.
///
/// When changing this, adapt ``hlt.execution_context`` in ``libhilti.ll``
/// and ``codegen::hlt::ExecutionContext`` in ``codegen/codegen.cc``.
struct __hlt_execution_context {
    __hlt_gchdr __gch;                /// Header for garbage collection.
    hlt_vthread_id vid;               /// The ID of the virtual thread this context belongs to. HLT_VID_MAIN for the main thread.
#if 0
    struct hlt_worker_thread* worker; /// The worker thread this virtual thread is mapped to. NULL for the main thread.
    hlt_continuation* yield;          /// A continuation to call when a ``yield`` statement is executed.
    hlt_continuation* resume;         /// A continuation to call when resuming after a ``yield`` statement. Is set by the ``yield``.

    /// We keep an array of callable registered for execution but not
    /// processed yet.
    uint64_t       calls_num;     /// Number of callables in array.
    uint64_t       calls_alloced; /// Number of slots allocated.
    uint64_t       calls_first;   /// Index of first non-yet processed element.
    hlt_callable** calls;         /// First element of array, or 0 if empty.

    hlt_profiling_state* pstate;  /// State for ongoing profiling, or 0 if none.
#endif

    // TODO; We should not compile this in non-debug mode.
    uint64_t debug_indent;        /// Current indent level for debug messages.

#if 0
    void* tcontext; /// The current threading context, per the module's "context" definition; NULL if not set.
#endif
    void *globals;  // The globals are starting right here (at the address of the field, the pointer content is ignored.)
};

/// Creates a new execution context.
///
/// vid: The ID of the virtual thread the context will belong to.
/// ~~HLT_VID_MAIN for the main (non-worker) thread.
///
/// Returns: The new context.
extern hlt_execution_context* __hlt_execution_context_new(hlt_vthread_id vid);

#endif
