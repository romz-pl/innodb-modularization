#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

    #if defined(HAVE_PAUSE_INSTRUCTION)
        /* According to the gcc info page, asm volatile means that the
        instruction has important side-effects and must not be removed.
        Also asm volatile may trigger a memory barrier (spilling all registers
        to memory). */
        #ifdef __SUNPRO_CC
            #define UT_RELAX_CPU() asm("pause")
        #else
            #define UT_RELAX_CPU() __asm__ __volatile__("pause")
        #endif /* __SUNPRO_CC */

    #elif defined(HAVE_FAKE_PAUSE_INSTRUCTION)
        #define UT_RELAX_CPU() __asm__ __volatile__("rep; nop")
    #elif defined _WIN32
        /* In the Win32 API, the x86 PAUSE instruction is executed by calling
        the YieldProcessor macro defined in WinNT.h. It is a CPU architecture-
        independent way by using YieldProcessor. */
        #define UT_RELAX_CPU() YieldProcessor()
    #else
        #define UT_RELAX_CPU() __asm__ __volatile__("" ::: "memory")
    #endif


#else                  /* !UNIV_HOTBACKUP */
    #define UT_RELAX_CPU() /* No op */
#endif                 /* !UNIV_HOTBACKUP */
