#pragma once

#include <innodb/univ/univ.h>


#ifdef UNIV_COMPILE_TEST_FUNCS

#ifdef HAVE_UT_CHRONO_T

void test_cmp_data_data(ulint len) {
  int i;
  static byte zeros[64];

  if (len > sizeof zeros) {
    len = sizeof zeros;
  }

  ut_chrono_t ch(__func__);

  for (i = 1000000; i > 0; i--) {
    i += cmp_data(DATA_INT, 0, zeros, len, zeros, len);
  }
}

#endif /* HAVE_UT_CHRONO_T */

#endif /* UNIV_COMPILE_TEST_FUNCS */
