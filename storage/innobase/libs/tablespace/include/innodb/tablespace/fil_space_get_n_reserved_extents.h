#pragma once

#include <innodb/univ/univ.h>



ulint fil_space_get_n_reserved_extents(space_id_t space_id)
    MY_ATTRIBUTE((warn_unused_result));
