#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_DEBUG

#include <innodb/memory/mem_heap_t.h>

void mem_heap_validate(const mem_heap_t *heap);

#endif
