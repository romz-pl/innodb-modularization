#pragma once

#include <innodb/univ/univ.h>

#include <innodb/tablespace/space_id_t.h>

bool fsp_is_file_per_table(space_id_t space_id, uint32_t fsp_flags);
