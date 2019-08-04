/*****************************************************************************

Copyright (c) 1997, 2018, Oracle and/or its affiliates. All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is also distributed with certain software (including but not
limited to OpenSSL) that is licensed under separate terms, as designated in a
particular file or component or in included license documentation. The authors
of MySQL hereby grant you an additional permission to link the program and
your derivative works with the separately licensed software that they have
included with MySQL.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/

/** @file include/hash0hash.h
 The simple hash table utility

 Created 5/20/1997 Heikki Tuuri
 *******************************************************/

#ifndef hash0hash_h
#define hash0hash_h

#include <innodb/univ/univ.h>
#include <innodb/sync_latch/latch_id_t.h>
#include <innodb/sync_mutex/ib_mutex_t.h>
#include <innodb/sync_rw/rw_lock_t.h>
#include <innodb/hash/hash_node_t.h>
#include <innodb/hash/hash_table_sync_t.h>
#include <innodb/hash/hash_cell_t.h>
#include <innodb/hash/hash_table_t.h>
#include <innodb/hash/hash_create.h>
#include <innodb/hash/hash_get_nth_cell.h>
#include <innodb/hash/hash_table_clear.h>
#include <innodb/hash/hash_calc_hash.h>
#include <innodb/hash/hash_get_n_cells.h>
#include <innodb/hash/hash_get_sync_obj_index.h>
#include <innodb/hash/hash_get_nth_heap.h>
#include <innodb/hash/hash_get_heap.h>
#include <innodb/hash/hash_get_nth_mutex.h>
#include <innodb/hash/hash_get_nth_lock.h>
#include <innodb/hash/hash_get_mutex.h>
#include <innodb/hash/hash_get_lock.h>
#include <innodb/hash/hash_lock_s_confirm.h>
#include <innodb/hash/hash_lock_x_confirm.h>

#include <innodb/math/ut_2pow_remainder.h>
#include <innodb/random/random.h>
#include <innodb/sync_rw/rw_lock_s_unlock.h>
#include <innodb/sync_rw/rw_lock_s_lock.h>
#include <innodb/sync_rw/rw_lock_x_unlock.h>
#include <innodb/sync_rw/rw_lock_x_lock.h>
#include <innodb/hash/hash_lock_x_all.h>
#include <innodb/hash/hash_unlock_x_all.h>
#include <innodb/hash/hash_unlock_x_all_but.h>
#include <innodb/hash/hash_table_free.h>
#include <innodb/hash/HASH_ASSERT_OWN.h>
#include <innodb/hash/HASH_DELETE.h>
#include <innodb/hash/HASH_INVALIDATE.h>
#include <innodb/hash/HASH_ASSERT_VALID.h>
#include <innodb/hash/HASH_INSERT.h>
#include <innodb/hash/hash_create_sync_obj.h>


#include <stddef.h>

#include "mem0mem.h"










/** Deletes a struct from a hash table. */


/** Gets the first struct in a hash chain, NULL if none. */

#define HASH_GET_FIRST(TABLE, HASH_VAL) \
  (hash_get_nth_cell(TABLE, HASH_VAL)->node)

/** Gets the next struct in a hash chain, NULL if none. */

#define HASH_GET_NEXT(NAME, DATA) ((DATA)->NAME)

/** Looks for a struct in a hash table. */
#define HASH_SEARCH(NAME, TABLE, FOLD, TYPE, DATA, ASSERTION, TEST)    \
  {                                                                    \
    HASH_ASSERT_OWN(TABLE, FOLD)                                       \
                                                                       \
    (DATA) = (TYPE)HASH_GET_FIRST(TABLE, hash_calc_hash(FOLD, TABLE)); \
    HASH_ASSERT_VALID(DATA);                                           \
                                                                       \
    while ((DATA) != NULL) {                                           \
      ASSERTION;                                                       \
      if (TEST) {                                                      \
        break;                                                         \
      } else {                                                         \
        HASH_ASSERT_VALID(HASH_GET_NEXT(NAME, DATA));                  \
        (DATA) = (TYPE)HASH_GET_NEXT(NAME, DATA);                      \
      }                                                                \
    }                                                                  \
  }

/** Looks for an item in all hash buckets. */
#define HASH_SEARCH_ALL(NAME, TABLE, TYPE, DATA, ASSERTION, TEST) \
  do {                                                            \
    ulint i3333;                                                  \
                                                                  \
    for (i3333 = (TABLE)->n_cells; i3333--;) {                    \
      (DATA) = (TYPE)HASH_GET_FIRST(TABLE, i3333);                \
                                                                  \
      while ((DATA) != NULL) {                                    \
        HASH_ASSERT_VALID(DATA);                                  \
        ASSERTION;                                                \
                                                                  \
        if (TEST) {                                               \
          break;                                                  \
        }                                                         \
                                                                  \
        (DATA) = (TYPE)HASH_GET_NEXT(NAME, DATA);                 \
      }                                                           \
                                                                  \
      if ((DATA) != NULL) {                                       \
        break;                                                    \
      }                                                           \
    }                                                             \
  } while (0)





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

#ifndef UNIV_HOTBACKUP
/** Move all hash table entries from OLD_TABLE to NEW_TABLE. */

#define HASH_MIGRATE(OLD_TABLE, NEW_TABLE, NODE_TYPE, PTR_NAME, FOLD_FUNC)  \
  do {                                                                      \
    ulint i2222;                                                            \
    ulint cell_count2222;                                                   \
                                                                            \
    cell_count2222 = hash_get_n_cells(OLD_TABLE);                           \
                                                                            \
    for (i2222 = 0; i2222 < cell_count2222; i2222++) {                      \
      NODE_TYPE *node2222 =                                                 \
          static_cast<NODE_TYPE *>(HASH_GET_FIRST((OLD_TABLE), i2222));     \
                                                                            \
      while (node2222) {                                                    \
        NODE_TYPE *next2222 = static_cast<NODE_TYPE *>(node2222->PTR_NAME); \
        ulint fold2222 = FOLD_FUNC(node2222);                               \
                                                                            \
        HASH_INSERT(NODE_TYPE, PTR_NAME, (NEW_TABLE), fold2222, node2222);  \
                                                                            \
        node2222 = next2222;                                                \
      }                                                                     \
    }                                                                       \
  } while (0)


#endif /* !UNIV_HOTBACKUP */



#endif
