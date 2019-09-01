#pragma once

#include <innodb/univ/univ.h>

struct lock_t;
struct hash_table_t;
struct buf_block_t;

/** Gets the first record lock on a page, where the page is identified by a
pointer to it.
@param[in]	lock_hash	lock hash table
@param[in]	block		buffer block
@return first lock, NULL if none exists */
UNIV_INLINE
lock_t *lock_rec_get_first_on_page(hash_table_t *lock_hash,
                                   const buf_block_t *block);
