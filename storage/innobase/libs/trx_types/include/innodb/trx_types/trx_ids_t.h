#pragma once

#include <innodb/univ/univ.h>

#include <innodb/allocator/ut_allocator.h>

#include <vector>

typedef std::vector<trx_id_t, ut_allocator<trx_id_t>> trx_ids_t;
