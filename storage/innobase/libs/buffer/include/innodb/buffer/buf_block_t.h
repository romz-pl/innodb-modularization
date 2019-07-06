#pragma once

#include <innodb/univ/univ.h>

#include <innodb/buffer/buf_page_t.h>
#include <innodb/buffer/BPageLock.h>
#include <innodb/buffer/BPageMutex.h>
#include <innodb/page/page_type_t.h>
#include <innodb/page/type.h>
#include <innodb/machine/data.h>

struct dict_index_t;



/** The buffer control block structure */
struct buf_block_t {
  /** @name General fields */
  /* @{ */

  buf_page_t page; /*!< page information; this must
                   be the first field, so that
                   buf_pool->page_hash can point
                   to buf_page_t or buf_block_t */
  byte *frame;     /*!< pointer to buffer frame which
                   is of size UNIV_PAGE_SIZE, and
                   aligned to an address divisible by
                   UNIV_PAGE_SIZE */
#ifndef UNIV_HOTBACKUP
  BPageLock lock; /*!< read-write lock of the buffer
                  frame */
#endif            /* UNIV_HOTBACKUP */
  UT_LIST_NODE_T(buf_block_t) unzip_LRU;
  /*!< node of the decompressed LRU list;
  a block is in the unzip_LRU list
  if page.state == BUF_BLOCK_FILE_PAGE
  and page.zip.data != NULL. Protected by
  both LRU_list_mutex and the block
  mutex. */
#ifdef UNIV_DEBUG
  ibool in_unzip_LRU_list; /*!< TRUE if the page is in the
                         decompressed LRU list;
                         used in debugging */
  ibool in_withdraw_list;
#endif                         /* UNIV_DEBUG */
  unsigned lock_hash_val : 32; /*!< hashed value of the page address
                              in the record lock hash table;
                              protected by buf_block_t::lock
                              (or buf_block_t::mutex in
                              buf_page_get_gen(),
                              buf_page_init_for_read()
                              and buf_page_create()) */
  /* @} */
  /** @name Optimistic search field */
  /* @{ */

  uint64_t modify_clock; /*!< this clock is incremented every
                            time a pointer to a record on the
                            page may become obsolete; this is
                            used in the optimistic cursor
                            positioning: if the modify clock has
                            not changed, we know that the pointer
                            is still valid; this field may be
                            changed if the thread (1) owns the LRU
                            list mutex and the page is not
                            bufferfixed, or (2) the thread has an
                            x-latch on the block, or (3) the block
                            must belong to an intrinsic table */
  /* @} */
  /** @name Hash search fields (unprotected)
  NOTE that these fields are NOT protected by any semaphore! */
  /* @{ */

  ulint n_hash_helps;      /*!< counter which controls building
                           of a new hash index for the page */
  volatile ulint n_bytes;  /*!< recommended prefix length for hash
                           search: number of bytes in
                           an incomplete last field */
  volatile ulint n_fields; /*!< recommended prefix length for hash
                           search: number of full fields */
  volatile bool left_side; /*!< true or false, depending on
                           whether the leftmost record of several
                           records with the same prefix should be
                           indexed in the hash index */
                           /* @} */

  /** @name Hash search fields
  These 5 fields may only be modified when:
  we are holding the appropriate x-latch in btr_search_latches[], and
  one of the following holds:
  (1) the block state is BUF_BLOCK_FILE_PAGE, and
  we are holding an s-latch or x-latch on buf_block_t::lock, or
  (2) buf_block_t::buf_fix_count == 0, or
  (3) the block state is BUF_BLOCK_REMOVE_HASH.

  An exception to this is when we init or create a page
  in the buffer pool in buf0buf.cc.

  Another exception for buf_pool_clear_hash_index() is that
  assigning block->index = NULL (and block->n_pointers = 0)
  is allowed whenever btr_search_own_all(RW_LOCK_X).

  Another exception is that ha_insert_for_fold_func() may
  decrement n_pointers without holding the appropriate latch
  in btr_search_latches[]. Thus, n_pointers must be
  protected by atomic memory access.

  This implies that the fields may be read without race
  condition whenever any of the following hold:
  - the btr_search_latches[] s-latch or x-latch is being held, or
  - the block state is not BUF_BLOCK_FILE_PAGE or BUF_BLOCK_REMOVE_HASH,
  and holding some latch prevents the state from changing to that.

  Some use of assert_block_ahi_empty() or assert_block_ahi_valid()
  is prone to race conditions while buf_pool_clear_hash_index() is
  executing (the adaptive hash index is being disabled). Such use
  is explicitly commented. */

  /* @{ */

#if defined UNIV_AHI_DEBUG || defined UNIV_DEBUG
  ulint n_pointers; /*!< used in debugging: the number of
                    pointers in the adaptive hash index
                    pointing to this frame;
                    protected by atomic memory access
                    or btr_search_own_all(). */
#define assert_block_ahi_empty(block) \
  ut_a(os_atomic_increment_ulint(&(block)->n_pointers, 0) == 0)
#define assert_block_ahi_empty_on_init(block)                        \
  do {                                                               \
    UNIV_MEM_VALID(&(block)->n_pointers, sizeof(block)->n_pointers); \
    assert_block_ahi_empty(block);                                   \
  } while (0)
#define assert_block_ahi_valid(block) \
  ut_a((block)->index ||              \
       os_atomic_increment_ulint(&(block)->n_pointers, 0) == 0)
#else                                         /* UNIV_AHI_DEBUG || UNIV_DEBUG */
#define assert_block_ahi_empty(block)         /* nothing */
#define assert_block_ahi_empty_on_init(block) /* nothing */
#define assert_block_ahi_valid(block)         /* nothing */
#endif                                        /* UNIV_AHI_DEBUG || UNIV_DEBUG */
  unsigned curr_n_fields : 10; /*!< prefix length for hash indexing:
                              number of full fields */
  unsigned curr_n_bytes : 15;  /*!< number of bytes in hash
                               indexing */
  unsigned curr_left_side : 1; /*!< TRUE or FALSE in hash indexing */
  dict_index_t *index;         /*!< Index for which the
                               adaptive hash index has been
                               created, or NULL if the page
                               does not exist in the
                               index. Note that it does not
                               guarantee that the index is
                               complete, though: there may
                               have been hash collisions,
                               record deletions, etc. */
  /* @} */
  bool made_dirty_with_no_latch;
  /*!< true if block has been made dirty
  without acquiring X/SX latch as the
  block belongs to temporary tablespace
  and block is always accessed by a
  single thread. */
#ifndef UNIV_HOTBACKUP
#ifdef UNIV_DEBUG
  /** @name Debug fields */
  /* @{ */
  rw_lock_t debug_latch; /*!< in the debug version, each thread
                         which bufferfixes the block acquires
                         an s-latch here; so we can use the
                         debug utilities in sync0rw */
                         /* @} */
#endif                   /* UNIV_DEBUG */
#endif                   /* !UNIV_HOTBACKUP */
  BPageMutex mutex;      /*!< mutex protecting this block:
                         state (also protected by the buffer
                         pool mutex), io_fix, buf_fix_count,
                         and accessed; we introduce this new
                         mutex in InnoDB-5.1 to relieve
                         contention on the buffer pool mutex */

  /** Get the page number of the current buffer block.
  @return page number of the current buffer block. */
  page_no_t get_page_no() const { return (page.id.page_no()); }

  /** Get the next page number of the current buffer block.
  @return next page number of the current buffer block. */
  page_no_t get_next_page_no() const {
    return (mach_read_from_4(frame + FIL_PAGE_NEXT));
  }

  /** Get the page type of the current buffer block.
  @return page type of the current buffer block. */
  page_type_t get_page_type() const {
    return (mach_read_from_2(frame + FIL_PAGE_TYPE));
  }

  /** Get the page type of the current buffer block as string.
  @return page type of the current buffer block as string. */
  const char *get_page_type_str() const;
};
