#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/hash/hash_get_n_cells.h>
#include <innodb/hash/HASH_GET_FIRST.h>
#include <innodb/hash/HASH_INSERT.h>

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
