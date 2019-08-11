#pragma once

#include <innodb/univ/univ.h>

class buf_page_t;
struct buf_block_t;
buf_block_t *buf_page_get_block(buf_page_t *bpage);
