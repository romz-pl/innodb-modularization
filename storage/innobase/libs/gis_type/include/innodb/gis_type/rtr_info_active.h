#pragma once

#include <innodb/univ/univ.h>

#include <innodb/allocator/ut_allocator.h>

#include <list>

struct rtr_info_t;

typedef std::list<rtr_info_t *, ut_allocator<rtr_info_t *>> rtr_info_active;
