#pragma once

#include <innodb/univ/univ.h>

#include <innodb/tablespace/space_id_t.h>

char *fil_space_get_first_path(space_id_t space_id) MY_ATTRIBUTE((warn_unused_result));
