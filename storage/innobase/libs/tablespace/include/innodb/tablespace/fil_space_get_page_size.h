#pragma once

#include <innodb/univ/univ.h>


#include <innodb/disk/page_size_t.h>

const page_size_t fil_space_get_page_size(space_id_t space_id, bool *found)
    MY_ATTRIBUTE((warn_unused_result));
