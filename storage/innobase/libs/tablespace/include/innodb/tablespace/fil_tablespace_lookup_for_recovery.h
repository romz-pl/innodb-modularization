#pragma once

#include <innodb/univ/univ.h>



bool fil_tablespace_lookup_for_recovery(space_id_t space_id)
    MY_ATTRIBUTE((warn_unused_result));
