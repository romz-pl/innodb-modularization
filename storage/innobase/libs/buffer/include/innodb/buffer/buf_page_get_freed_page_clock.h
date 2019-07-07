#pragma once

#include <innodb/univ/univ.h>

class buf_page_t;
ulint buf_page_get_freed_page_clock(const buf_page_t *bpage);
