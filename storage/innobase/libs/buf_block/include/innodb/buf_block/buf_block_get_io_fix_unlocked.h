#pragma once

#include <innodb/univ/univ.h>

#include <innodb/buf_page/buf_io_fix.h>

struct buf_block_t;
buf_io_fix buf_block_get_io_fix_unlocked(const buf_block_t *block);
