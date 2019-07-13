#pragma once

#include <innodb/univ/univ.h>

#include <innodb/tablespace/space_id_t.h>

bool fil_tablespace_open_for_recovery(space_id_t space_id)
    MY_ATTRIBUTE((warn_unused_result));
