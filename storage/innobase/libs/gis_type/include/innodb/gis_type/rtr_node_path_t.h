#pragma once

#include <innodb/univ/univ.h>

#include <innodb/gis_type/node_visit_t.h>
#include <innodb/allocator/ut_allocator.h>

#include <vector>

typedef std::vector<node_visit_t, ut_allocator<node_visit_t>> rtr_node_path_t;
