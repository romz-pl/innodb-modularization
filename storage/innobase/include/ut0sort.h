/*****************************************************************************

Copyright (c) 1995, 2018, Oracle and/or its affiliates. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is also distributed with certain software (including but not
limited to OpenSSL) that is licensed under separate terms, as designated in a
particular file or component or in included license documentation. The authors
of MySQL hereby grant you an additional permission to link the program and
your derivative works with the separately licensed software that they have
included with MySQL.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/

/** @file include/ut0sort.h
 Sort utility

 Created 11/9/1995 Heikki Tuuri
 ***********************************************************************/

#ifndef ut0sort_h
#define ut0sort_h

#include <innodb/univ/univ.h>

/* This module gives a macro definition of the body of
a standard sort function for an array of elements of any
type. The comparison function is given as a parameter to
the macro. The sort algorithm is mergesort which has logarithmic
worst case.
*/

/** This macro expands to the body of a standard sort function.
 The sort function uses mergesort and must be defined separately
 for each type of array.
 Also the comparison function has to be defined individually
 for each array cell type. SORT_FUN is the sort function name.
 The function takes the array to be sorted (ARR),
 the array of auxiliary space (AUX_ARR) of same size,
 and the low (LOW), inclusive, and high (HIGH), noninclusive,
 limits for the sort interval as arguments.
 CMP_FUN is the comparison function name. It takes as arguments
 two elements from the array and returns 1, if the first is bigger,
 0 if equal, and -1 if the second bigger. */

#define UT_SORT_FUNCTION_BODY(SORT_FUN, ARR, AUX_ARR, LOW, HIGH, CMP_FUN)    \
  {                                                                          \
    ulint ut_sort_mid77;                                                     \
    ulint ut_sort_i77;                                                       \
    ulint ut_sort_low77;                                                     \
    ulint ut_sort_high77;                                                    \
                                                                             \
    ut_ad((LOW) < (HIGH));                                                   \
    ut_ad(ARR);                                                              \
    ut_ad(AUX_ARR);                                                          \
                                                                             \
    if ((LOW) == (HIGH)-1) {                                                 \
      return;                                                                \
    } else if ((LOW) == (HIGH)-2) {                                          \
      if (CMP_FUN((ARR)[LOW], (ARR)[(HIGH)-1]) > 0) {                        \
        (AUX_ARR)[LOW] = (ARR)[LOW];                                         \
        (ARR)[LOW] = (ARR)[(HIGH)-1];                                        \
        (ARR)[(HIGH)-1] = (AUX_ARR)[LOW];                                    \
      }                                                                      \
      return;                                                                \
    }                                                                        \
                                                                             \
    ut_sort_mid77 = ((LOW) + (HIGH)) / 2;                                    \
                                                                             \
    SORT_FUN((ARR), (AUX_ARR), (LOW), ut_sort_mid77);                        \
    SORT_FUN((ARR), (AUX_ARR), ut_sort_mid77, (HIGH));                       \
                                                                             \
    ut_sort_low77 = (LOW);                                                   \
    ut_sort_high77 = ut_sort_mid77;                                          \
                                                                             \
    for (ut_sort_i77 = (LOW); ut_sort_i77 < (HIGH); ut_sort_i77++) {         \
      if (ut_sort_low77 >= ut_sort_mid77) {                                  \
        (AUX_ARR)[ut_sort_i77] = (ARR)[ut_sort_high77];                      \
        ut_sort_high77++;                                                    \
      } else if (ut_sort_high77 >= (HIGH)) {                                 \
        (AUX_ARR)[ut_sort_i77] = (ARR)[ut_sort_low77];                       \
        ut_sort_low77++;                                                     \
      } else if (CMP_FUN((ARR)[ut_sort_low77], (ARR)[ut_sort_high77]) > 0) { \
        (AUX_ARR)[ut_sort_i77] = (ARR)[ut_sort_high77];                      \
        ut_sort_high77++;                                                    \
      } else {                                                               \
        (AUX_ARR)[ut_sort_i77] = (ARR)[ut_sort_low77];                       \
        ut_sort_low77++;                                                     \
      }                                                                      \
    }                                                                        \
                                                                             \
    memcpy((void *)((ARR) + (LOW)), (AUX_ARR) + (LOW),                       \
           ((HIGH) - (LOW)) * sizeof *(ARR));                                \
  }

#endif
