#pragma once

#include <innodb/univ/univ.h>

class buf_page_t;
struct buf_block_t;

ulint buf_block_fix(buf_page_t *bpage);
ulint buf_block_fix(buf_block_t *block);
