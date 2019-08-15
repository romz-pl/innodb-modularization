#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_types/index_id_t.h>
#include <innodb/allocator/ut_allocator.h>

#include <vector>

typedef std::vector<index_id_t, ut_allocator<index_id_t>> corrupted_ids_t;
