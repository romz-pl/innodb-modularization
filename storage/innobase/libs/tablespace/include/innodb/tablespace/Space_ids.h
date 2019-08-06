#pragma once

#include <innodb/univ/univ.h>


#include <innodb/allocator/ut_allocator.h>

#include <vector>

using Space_ids = std::vector<space_id_t, ut_allocator<space_id_t>>;
