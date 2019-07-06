#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>

bool mem_heap_is_top(mem_heap_t *heap, const void *buf, ulint buf_sz);
