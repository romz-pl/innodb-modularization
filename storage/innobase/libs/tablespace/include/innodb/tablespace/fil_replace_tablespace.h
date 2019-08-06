#pragma once

#include <innodb/univ/univ.h>


#include <innodb/disk/page_no_t.h>

bool fil_replace_tablespace(space_id_t old_space_id, space_id_t new_space_id,
                            page_no_t size_in_pages)
    MY_ATTRIBUTE((warn_unused_result));
