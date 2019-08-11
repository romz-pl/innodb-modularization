#pragma once

#include <innodb/univ/univ.h>

#include <innodb/buf_frame/buf_frame_t.h>

struct buf_block_t;
buf_frame_t *buf_block_get_frame(const buf_block_t *block);
