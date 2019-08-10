#pragma once

#include <innodb/univ/univ.h>

struct fil_node_t;

bool is_ddl_temp_table(fil_node_t *node);
