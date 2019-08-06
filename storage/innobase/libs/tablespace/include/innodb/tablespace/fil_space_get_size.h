#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_no_t.h>


page_no_t fil_space_get_size(space_id_t space_id)
    MY_ATTRIBUTE((warn_unused_result));
