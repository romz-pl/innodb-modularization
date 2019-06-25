#include <innodb/wait/ut_delay.h>
#include <innodb/wait/UT_RELAX_CPU.h>
#include <innodb/wait/spin_wait_pause_multiplier.h>


#ifndef UNIV_HOTBACKUP

#if defined(HAVE_HMT_PRIORITY_INSTRUCTION)
    #define UT_LOW_PRIORITY_CPU() __asm__ __volatile__("or 1,1,1")
    #define UT_RESUME_PRIORITY_CPU() __asm__ __volatile__("or 2,2,2")
#else
    #define UT_LOW_PRIORITY_CPU() ((void)0)
    #define UT_RESUME_PRIORITY_CPU() ((void)0)
#endif


ulint ut_delay(ulint delay) {
  ulint i, j;
  /* We don't expect overflow here, as ut::spin_wait_pause_multiplier is limited
  to 100, and values of delay are not larger than @@innodb_spin_wait_delay
  which is limited by 1 000. Anyway, in case an overflow happened, the program
  would still work (as iterations is unsigned). */
  const ulint iterations = delay * ut::spin_wait_pause_multiplier;
  UT_LOW_PRIORITY_CPU();

  j = 0;

  for (i = 0; i < iterations; i++) {
    j += i;
    UT_RELAX_CPU();
  }

  UT_RESUME_PRIORITY_CPU();

  return (j);
}

#endif
