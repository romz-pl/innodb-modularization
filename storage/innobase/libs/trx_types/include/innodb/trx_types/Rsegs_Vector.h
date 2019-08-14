#pragma once

#include <innodb/univ/univ.h>

#include <innodb/allocator/ut_allocator.h>

#include <vector>

struct trx_rseg_t;

using Rsegs_Vector = std::vector<trx_rseg_t *, ut_allocator<trx_rseg_t *>>;
