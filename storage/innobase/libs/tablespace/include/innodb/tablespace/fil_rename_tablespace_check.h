#pragma once

#include <innodb/univ/univ.h>


#include <innodb/error/dberr_t.h>

dberr_t fil_rename_tablespace_check(space_id_t space_id, const char *old_path,
                                    const char *new_path, bool is_discarded)
    MY_ATTRIBUTE((warn_unused_result));
