#pragma once

#include <innodb/univ/univ.h>

#include <innodb/page/page_t.h>

uint32 fsp_header_get_space_version(const page_t *page);
