#pragma once

#include <innodb/univ/univ.h>

#include <innodb/buffer/buf_page_state.h>

struct buf_block_t;
void buf_block_set_state(buf_block_t *block, buf_page_state state);
