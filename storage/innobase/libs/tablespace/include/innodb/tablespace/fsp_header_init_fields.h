#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>
#include <innodb/tablespace/space_id_t.h>

/** Writes the space id and flags to a tablespace header.  The flags contain
 row type, physical/compressed page size, and logical/uncompressed page
 size of the tablespace. */
void fsp_header_init_fields(
    page_t *page,        /*!< in/out: first page in the space */
    space_id_t space_id, /*!< in: space id */
    uint32_t flags);     /*!< in: tablespace flags
                         (FSP_SPACE_FLAGS): 0, or
                         table->flags if newer than COMPACT */
