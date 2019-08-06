#pragma once

#include <innodb/univ/univ.h>



struct fil_space_t;

fil_space_t *fil_space_acquire_silent(space_id_t space_id)
    MY_ATTRIBUTE((warn_unused_result));
