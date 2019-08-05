#pragma once

#include <innodb/univ/univ.h>

#include <innodb/tablespace/space_id_t.h>
#include <innodb/tablespace/fil_type_t.h>
#include <innodb/disk/page_no_t.h>
#include <innodb/error/dberr_t.h>

dberr_t fil_create_tablespace(space_id_t space_id, const char *name,
                                     const char *path, uint32_t flags,
                                     page_no_t size, fil_type_t type);
