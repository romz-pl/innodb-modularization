#pragma once

#include <innodb/univ/univ.h>

struct buf_block_t;
void buf_block_buf_fix_dec(buf_block_t *block);
