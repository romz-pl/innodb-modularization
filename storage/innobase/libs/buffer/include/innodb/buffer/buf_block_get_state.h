#pragma once

#include <innodb/univ/univ.h>

#include <innodb/buffer/buf_page_state.h>

struct buf_block_t;
buf_page_state buf_block_get_state(const buf_block_t *block);
