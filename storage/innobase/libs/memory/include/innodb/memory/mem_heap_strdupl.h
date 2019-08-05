#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>

/** Makes a NUL-terminated copy of a nonterminated string, allocated from a
memory heap.
@param[in]	heap	memory heap where string is allocated
@param[in]	str	string to be copied
@param[in]	len	length of str, in bytes
@return own: a copy of the string */
char *mem_heap_strdupl(mem_heap_t *heap, const char *str, ulint len);
