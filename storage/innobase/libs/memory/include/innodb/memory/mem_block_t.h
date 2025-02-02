#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_block_info_t.h>

/** A block of a memory heap consists of the info structure
followed by an area of memory */
typedef struct mem_block_info_t mem_block_t;
