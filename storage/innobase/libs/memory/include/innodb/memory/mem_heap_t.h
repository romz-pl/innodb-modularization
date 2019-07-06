#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_block_t.h>

/** A memory heap is a nonempty linear list of memory blocks */
typedef mem_block_t mem_heap_t;
