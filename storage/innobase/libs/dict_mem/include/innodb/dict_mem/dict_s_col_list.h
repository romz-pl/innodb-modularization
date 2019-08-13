#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_s_col_t.h>
#include <innodb/allocator/ut_allocator.h>

#include <list>

/** list to put stored column for dict_table_t */
typedef std::list<dict_s_col_t, ut_allocator<dict_s_col_t>> dict_s_col_list;
