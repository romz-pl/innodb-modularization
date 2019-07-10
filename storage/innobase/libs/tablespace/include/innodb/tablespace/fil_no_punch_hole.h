#pragma once

#include <innodb/univ/univ.h>

struct fil_node_t;

void fil_no_punch_hole(fil_node_t *file);
