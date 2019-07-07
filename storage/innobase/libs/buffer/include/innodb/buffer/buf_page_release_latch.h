#pragma once

#include <innodb/univ/univ.h>

struct buf_block_t;
void buf_page_release_latch(buf_block_t *block, ulint rw_latch);
