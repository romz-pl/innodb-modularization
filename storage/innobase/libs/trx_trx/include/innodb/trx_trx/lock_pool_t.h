#pragma once

#include <innodb/univ/univ.h>

#include <innodb/allocator/ut_allocator.h>

#include <vector>

struct lock_t;

typedef std::vector<lock_t *, ut_allocator<lock_t *>> lock_pool_t;
