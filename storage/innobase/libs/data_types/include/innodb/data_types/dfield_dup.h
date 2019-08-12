#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>

struct dfield_t;

/** Copies the data pointed to by a data field.
@param[in,out]	field	data field
@param[in]	heap	memory heap where allocated */
void dfield_dup(dfield_t *field, mem_heap_t *heap);
