#pragma once

#include <innodb/univ/univ.h>

#include <innodb/tablespace/space_id_t.h>

bool fil_space_reserve_free_extents(space_id_t space_id, ulint n_free_now,
                                    ulint n_to_reserve)
    MY_ATTRIBUTE((warn_unused_result));
