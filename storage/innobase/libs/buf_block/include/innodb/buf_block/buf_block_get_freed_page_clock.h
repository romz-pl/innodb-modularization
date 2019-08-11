#pragma once

#include <innodb/univ/univ.h>

struct buf_block_t;
ulint buf_block_get_freed_page_clock(const buf_block_t *block);
