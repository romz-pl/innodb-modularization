#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_foreign_t.h>
#include <innodb/string/ut_strcmp.h>

/** Compare two dict_foreign_t objects using their ids. Used in the ordering
of dict_table_t::foreign_set and dict_table_t::referenced_set.  It returns
true if the first argument is considered to go before the second in the
strict weak ordering it defines, and false otherwise. */
struct dict_foreign_compare {
  bool operator()(const dict_foreign_t *lhs, const dict_foreign_t *rhs) const {
    return (ut_strcmp(lhs->id, rhs->id) < 0);
  }
};
