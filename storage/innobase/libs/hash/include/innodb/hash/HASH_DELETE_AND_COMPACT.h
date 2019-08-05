#pragma once

#include <innodb/univ/univ.h>

#include <innodb/hash/HASH_DELETE.h>
#include <innodb/memory/mem_heap_get_top.h>
#include <innodb/hash/hash_get_nth_cell.h>
#include <innodb/hash/hash_calc_hash.h>
#include <innodb/hash/HASH_GET_NEXT.h>
// #include <innodb/memory/mem_heap_free_top.h>
#include <innodb/hash/hash_get_heap.h>

void mem_heap_free_top(mem_heap_t *heap, ulint n);

/** Deletes a struct which is stored in the heap of the hash table, and compacts
 the heap. The fold value must be stored in the struct NODE in a field named
 'fold'. */

#define HASH_DELETE_AND_COMPACT(TYPE, NAME, TABLE, NODE)                       \
  do {                                                                         \
    TYPE *node111;                                                             \
    TYPE *top_node111;                                                         \
    hash_cell_t *cell111;                                                      \
    ulint fold111;                                                             \
                                                                               \
    fold111 = (NODE)->fold;                                                    \
                                                                               \
    HASH_DELETE(TYPE, NAME, TABLE, fold111, NODE);                             \
                                                                               \
    top_node111 =                                                              \
        (TYPE *)mem_heap_get_top(hash_get_heap(TABLE, fold111), sizeof(TYPE)); \
                                                                               \
    /* If the node to remove is not the top node in the heap, compact the      \
    heap of nodes by moving the top node in the place of NODE. */              \
                                                                               \
    if (NODE != top_node111) {                                                 \
      /* Copy the top node in place of NODE */                                 \
                                                                               \
      *(NODE) = *top_node111;                                                  \
                                                                               \
      cell111 =                                                                \
          hash_get_nth_cell(TABLE, hash_calc_hash(top_node111->fold, TABLE));  \
                                                                               \
      /* Look for the pointer to the top node, to update it */                 \
                                                                               \
      if (cell111->node == top_node111) {                                      \
        /* The top node is the first in the chain */                           \
                                                                               \
        cell111->node = NODE;                                                  \
      } else {                                                                 \
        /* We have to look for the predecessor of the top                      \
        node */                                                                \
        node111 = static_cast<TYPE *>(cell111->node);                          \
                                                                               \
        while (top_node111 != HASH_GET_NEXT(NAME, node111)) {                  \
          node111 = static_cast<TYPE *>(HASH_GET_NEXT(NAME, node111));         \
        }                                                                      \
                                                                               \
        /* Now we have the predecessor node */                                 \
                                                                               \
        node111->NAME = NODE;                                                  \
      }                                                                        \
    }                                                                          \
                                                                               \
    /* Free the space occupied by the top node */                              \
                                                                               \
    mem_heap_free_top(hash_get_heap(TABLE, fold111), sizeof(TYPE));            \
  } while (0)
