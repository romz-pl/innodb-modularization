#pragma once

#include <innodb/univ/univ.h>

class buf_page_t;
unsigned buf_page_is_accessed(const buf_page_t *bpage);
