#pragma once

#include <innodb/univ/univ.h>

#include <innodb/tablespace/space_id_t.h>

bool fsp_is_session_temporary(space_id_t space_id);
