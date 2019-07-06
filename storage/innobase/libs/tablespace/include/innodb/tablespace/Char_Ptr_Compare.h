#pragma once

#include <innodb/univ/univ.h>

#include <string.h>

/** Compare two 'strings' */
struct Char_Ptr_Compare {
  /** Compare two NUL terminated strings
  @param[in]	lhs		Left hand side
  @param[in]	rhs		Right hand side
  @return true if the contents match */
  bool operator()(const char *lhs, const char *rhs) const {
    return (strcmp(lhs, rhs) == 0);
  }
};
