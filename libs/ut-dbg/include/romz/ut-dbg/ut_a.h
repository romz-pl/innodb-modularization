#pragma once

#include <romz/ut-dbg/ut_dbg_assertion_failed.h>

/** Abort execution if EXPR does not evaluate to nonzero.
@param EXPR assertion expression that should hold */
#define ut_a(EXPR)                                               \
  do {                                                           \
    if (!(uint64_t)(EXPR)) {                                     \
      ut_dbg_assertion_failed(#EXPR, __FILE__, (ulint)__LINE__); \
    }                                                            \
  } while (0)
