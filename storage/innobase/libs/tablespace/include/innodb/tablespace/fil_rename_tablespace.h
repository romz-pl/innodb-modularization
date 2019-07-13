#pragma once

#include <innodb/univ/univ.h>

#include <innodb/tablespace/space_id_t.h>
#include <innodb/error/dberr_t.h>

dberr_t fil_rename_tablespace(space_id_t space_id, const char *old_path,
                              const char *new_name, const char *new_path_in)
    MY_ATTRIBUTE((warn_unused_result));
