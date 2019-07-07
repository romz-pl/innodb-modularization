#pragma once

#include <innodb/univ/univ.h>

#include <innodb/buffer/buf_frame_t.h>

buf_frame_t *buf_frame_align(byte *ptr);
