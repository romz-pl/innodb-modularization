#pragma once

#include <innodb/univ/univ.h>

#include <innodb/random/random.h>

/** Hash a NUL terminated 'string' */
struct Char_Ptr_Hash {
  /** Hashing function
  @param[in]	ptr		NUL terminated string to hash
  @return the hash */
  size_t operator()(const char *ptr) const { return (ut_fold_string(ptr)); }
};
