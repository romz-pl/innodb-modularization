#pragma once

#include <innodb/univ/univ.h>

#include <innodb/page/page_size_t.h>
#include <innodb/page/page_t.h>

page_size_t fsp_header_get_page_size(const page_t *page);
