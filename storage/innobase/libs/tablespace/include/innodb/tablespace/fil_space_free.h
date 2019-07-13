#pragma once

#include <innodb/univ/univ.h>

#include <innodb/tablespace/space_id_t.h>

bool fil_space_free(space_id_t space_id, bool x_latched);
