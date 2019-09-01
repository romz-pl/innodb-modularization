#pragma once

#include <innodb/univ/univ.h>

struct lock_t;
struct hash_table_t;
struct RecID;
struct buf_block_t;

/** Gets the first explicit lock request on a record.
@param[in]	hash		Record hash
@param[in]	rec_id		Record ID
@return	first lock, nullptr if none exists */
UNIV_INLINE
lock_t *lock_rec_get_first(hash_table_t *hash, const RecID &rec_id);


/** Gets the first explicit lock request on a record.
@param[in]	hash	hash chain the lock on
@param[in]	block	block containing the record
@param[in]	heap_no	heap number of the record
@return first lock, NULL if none exists */
UNIV_INLINE
lock_t *lock_rec_get_first(hash_table_t *hash, const buf_block_t *block,
                           ulint heap_no);
