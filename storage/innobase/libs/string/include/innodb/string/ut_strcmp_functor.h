#pragma once

#include <innodb/univ/univ.h>
#include <string.h>

/** Functor that compares two C strings. Can be used as a comparator for
e.g. std::map that uses char* as keys. */
struct ut_strcmp_functor {
  bool operator()(const char *a, const char *b) const {
    return (strcmp(a, b) < 0);
  }
};
