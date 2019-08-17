#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

/** Gets the tablespace identifier.
 @return space id */
space_id_t page_get_space_id(const page_t *page); /*!< in: page */
