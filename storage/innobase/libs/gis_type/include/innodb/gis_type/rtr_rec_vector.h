#pragma once

#include <innodb/univ/univ.h>

#include <innodb/gis_type/rtr_rec_t.h>
#include <innodb/allocator/ut_allocator.h>

#include <vector>

typedef std::vector<rtr_rec_t, ut_allocator<rtr_rec_t>> rtr_rec_vector;
