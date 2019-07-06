#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>

void *mem_heap_get_top(mem_heap_t *heap, ulint n);
