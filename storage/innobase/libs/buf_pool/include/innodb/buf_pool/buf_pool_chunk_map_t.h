#pragma once

#include <innodb/univ/univ.h>

#include <innodb/allocator/ut_allocator.h>

#include <map>

struct buf_chunk_t;

/** Map of buffer pool chunks by its first frame address
This is newly made by initialization of buffer pool and buf_resize_thread.
Note: mutex protection is required when creating multiple buffer pools
in parallel. We don't use a mutex during resize because that is still single
threaded. */
typedef std::map<const byte *, buf_chunk_t *, std::less<const byte *>,
                 ut_allocator<std::pair<const byte *const, buf_chunk_t *>>>
    buf_pool_chunk_map_t;
