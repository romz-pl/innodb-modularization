#include <innodb/buffer/buf_block_get_lock_hash_val.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/buffer/buf_block_t.h>
#include <innodb/buffer/buf_page_in_file.h>
#include <innodb/sync_rw/rw_lock_own.h>

/** Gets the hash value of the page the pointer is pointing to. This can be used
 in searches in the lock hash table.
 @return lock hash value */
ulint buf_block_get_lock_hash_val(const buf_block_t *block) /*!< in: block */
{
  ut_ad(block);
  ut_ad(buf_page_in_file(&block->page));
  ut_ad(rw_lock_own(&(((buf_block_t *)block)->lock), RW_LOCK_X) ||
        rw_lock_own(&(((buf_block_t *)block)->lock), RW_LOCK_S));

  return (block->lock_hash_val);
}

#endif
