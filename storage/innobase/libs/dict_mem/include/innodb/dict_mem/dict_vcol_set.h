#pragma once

#include <innodb/univ/univ.h>

#include <innodb/allocator/ut_allocator.h>

#include <set>

struct dict_v_col_t;

/** Set to store the virtual columns which are affected by Foreign
key constraint. */
typedef std::set<dict_v_col_t *, std::less<dict_v_col_t *>,
                 ut_allocator<dict_v_col_t *>>
    dict_vcol_set;
