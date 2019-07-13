#pragma once

#include <innodb/univ/univ.h>

#include <innodb/tablespace/space_id_t.h>
#include <innodb/tablespace/buf_remove_t.h>
#include <innodb/error/dberr_t.h>

dberr_t fil_delete_tablespace(space_id_t space_id, buf_remove_t buf_remove)
    MY_ATTRIBUTE((warn_unused_result));
