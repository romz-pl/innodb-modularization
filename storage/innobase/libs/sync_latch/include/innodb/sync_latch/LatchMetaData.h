#pragma once

#include <innodb/univ/univ.h>
#include <innodb/allocator/ut_allocator.h>
#include <innodb/sync_latch/latch_meta_t.h>

#include <vector>

#ifndef UNIV_LIBRARY

typedef std::vector<latch_meta_t *, ut_allocator<latch_meta_t *>> LatchMetaData;

#endif
