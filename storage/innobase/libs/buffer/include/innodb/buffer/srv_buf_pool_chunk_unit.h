#pragma once

#include <innodb/univ/univ.h>

/** Requested buffer pool chunk size. Each buffer pool instance consists
of one or more chunks. */
extern ulonglong srv_buf_pool_chunk_unit;
