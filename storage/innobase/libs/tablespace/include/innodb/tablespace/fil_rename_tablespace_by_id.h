#pragma once

#include <innodb/univ/univ.h>

#include <innodb/tablespace/space_id_t.h>
#include <innodb/error/dberr_t.h>

dberr_t fil_rename_tablespace_by_id(space_id_t space_id, const char *old_name,
                                    const char *new_name)
    MY_ATTRIBUTE((warn_unused_result));
