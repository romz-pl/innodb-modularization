#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>


/** Configure the zlib allocator to use the given memory heap. */
void page_zip_set_alloc(void *stream,      /*!< in/out: zlib stream */
                        mem_heap_t *heap); /*!< in: memory heap to use */
