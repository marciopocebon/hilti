
#include <locale.h>
#include <stdio.h>
#include <stdlib.h>

#include "types.h"
#include "config.h"
#include "debug.h"
#include "globals.h"

// This is generated by the HILTI linker. Calling it initialized all the
// compiled modules.
extern void __hlt_modules_init(hlt_execution_context* ctx);

void hlt_init()
{
    // Initialize locale.
    if ( ! setlocale(LC_CTYPE, "") ) {
        fputs("libhilti: cannot set locale", stderr);
        exit(1);
    }

    // Initialize configuration to defaults.
    __hlt_config_init();

    // Initialize debugging system.
    __hlt_debug_init();

    // Initialize profiling if requested.
//  __hlt_profiling_init();

    // Initialize global library state.
    __hlt_global_state_init();
}

void hlt_done()
{
    __hlt_global_state_done();
//  __hlt_profiling_done();
    __hlt_debug_done();
    __hlt_config_done();
}

