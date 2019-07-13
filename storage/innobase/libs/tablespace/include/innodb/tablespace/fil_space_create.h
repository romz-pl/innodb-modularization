#pragma once

#include <innodb/univ/univ.h>

#include <innodb/tablespace/space_id_t.h>
#include <innodb/tablespace/fil_type_t.h>

struct fil_space_t;

fil_space_t *fil_space_create(const char *name, space_id_t space_id,
                              uint32_t flags, fil_type_t purpose)
    MY_ATTRIBUTE((warn_unused_result));
