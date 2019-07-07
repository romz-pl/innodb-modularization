#pragma once

#include <innodb/univ/univ.h>

#include <innodb/buffer/buf_io_fix.h>

struct buf_block_t;
void buf_block_set_io_fix(buf_block_t *block, buf_io_fix io_fix);
