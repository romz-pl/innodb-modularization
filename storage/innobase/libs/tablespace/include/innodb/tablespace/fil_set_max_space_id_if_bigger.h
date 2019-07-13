#pragma once

#include <innodb/univ/univ.h>

#include <innodb/tablespace/space_id_t.h>

void fil_set_max_space_id_if_bigger(space_id_t max_id);
