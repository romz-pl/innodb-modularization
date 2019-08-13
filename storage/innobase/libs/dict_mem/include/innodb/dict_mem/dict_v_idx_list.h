#pragma once

#include <innodb/univ/univ.h>

#include <innodb/allocator/ut_allocator.h>
#include <innodb/dict_mem/dict_v_idx_t.h>

#include <list>

/** Index list to put in dict_v_col_t */
typedef std::list<dict_v_idx_t, ut_allocator<dict_v_idx_t>> dict_v_idx_list;
