#pragma once

#include <innodb/univ/univ.h>

#include <innodb/tablespace/space_id_t.h>

space_id_t fil_space_get_id_by_name(const char *name)
    MY_ATTRIBUTE((warn_unused_result));
