#pragma once

#include <innodb/univ/univ.h>

#include <innodb/buf_frame/buf_frame_t.h>

byte *buf_frame_copy(byte *buf, const buf_frame_t *frame);
