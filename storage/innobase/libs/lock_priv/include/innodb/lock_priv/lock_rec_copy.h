#pragma once

#include <innodb/univ/univ.h>
#include <innodb/memory/mem_heap_t.h>

struct lock_t;

lock_t *lock_rec_copy(const lock_t *lock, /*!< in: record lock */
                             mem_heap_t *heap);
