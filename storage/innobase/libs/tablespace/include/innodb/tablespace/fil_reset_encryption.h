#pragma once

#include <innodb/univ/univ.h>

#include <innodb/tablespace/space_id_t.h>
#include <innodb/error/dberr_t.h>

dberr_t fil_reset_encryption(space_id_t space_id)
    MY_ATTRIBUTE((warn_unused_result));
