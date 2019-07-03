#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/Blocks.h>

/** Block collection */
extern Blocks *block_cache;

/** Number of blocks to allocate for sync read/writes */
const size_t MAX_BLOCKS = 128;

/** Block buffer size */
#define BUFFER_BLOCK_SIZE ((ulint)(UNIV_PAGE_SIZE * 1.3))
