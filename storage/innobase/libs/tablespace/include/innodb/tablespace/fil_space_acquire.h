#pragma once

#include <innodb/univ/univ.h>

#include <innodb/tablespace/space_id_t.h>

struct fil_space_t;

fil_space_t *fil_space_acquire(space_id_t space_id)
    MY_ATTRIBUTE((warn_unused_result));
