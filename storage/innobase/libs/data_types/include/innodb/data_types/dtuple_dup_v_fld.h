#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>

struct dtuple_t;

/** Duplicate the virtual field data in a dtuple_t
@param[in,out]		vrow	dtuple contains the virtual fields
@param[in]		heap	heap memory to use */
void dtuple_dup_v_fld(const dtuple_t *vrow, mem_heap_t *heap);
