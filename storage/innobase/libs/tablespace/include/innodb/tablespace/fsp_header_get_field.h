#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

uint32_t fsp_header_get_field(const page_t *page, ulint field);
