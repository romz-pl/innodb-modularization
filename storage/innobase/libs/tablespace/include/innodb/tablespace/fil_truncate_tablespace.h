#pragma once

#include <innodb/univ/univ.h>


#include <innodb/univ/page_no_t.h>

bool fil_truncate_tablespace(space_id_t space_id, page_no_t size_in_pages)
    MY_ATTRIBUTE((warn_unused_result));
