#pragma once

#include <innodb/univ/univ.h>




uint32_t fil_space_get_flags(space_id_t space_id)
    MY_ATTRIBUTE((warn_unused_result));
