#pragma once

#include <innodb/univ/univ.h>

#include <innodb/page/page_t.h>

ulint fsp_header_get_flags(const page_t *page);
