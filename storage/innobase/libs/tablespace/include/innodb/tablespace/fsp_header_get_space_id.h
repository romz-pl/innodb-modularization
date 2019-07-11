#pragma once

#include <innodb/univ/univ.h>

#include <innodb/tablespace/space_id_t.h>
#include <innodb/page/page_t.h>

space_id_t fsp_header_get_space_id(const page_t *page);
