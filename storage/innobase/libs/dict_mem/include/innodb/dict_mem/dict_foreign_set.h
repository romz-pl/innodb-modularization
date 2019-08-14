#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_foreign_compare.h>

#include <set>
#include <iosfwd>


typedef std::set<dict_foreign_t *, dict_foreign_compare,
                 ut_allocator<dict_foreign_t *>>
    dict_foreign_set;

std::ostream &operator<<(std::ostream &out, const dict_foreign_set &fk_set);
