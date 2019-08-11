#pragma once

#include <innodb/univ/univ.h>

#include <innodb/buffer/BufListMutex.h>
#include <innodb/buffer/BufPoolZipMutex.h>
#include <innodb/buffer/FlushHp.h>
#include <innodb/buffer/LRUHp.h>
#include <innodb/buffer/LRUItr.h>
#include <innodb/buf_block/buf_block_t.h>
#include <innodb/buffer/buf_buddy_free_t.h>
#include <innodb/buffer/buf_buddy_stat_t.h>
#include <innodb/buf_page/buf_flush_t.h>
#include <innodb/buf_page/buf_page_t.h>
#include <innodb/buffer/buf_pool_stat_t.h>
#include <innodb/buf_page/flags.h>
#include <innodb/lst/lst.h>
#include <innodb/disk/page_no_t.h>
#include <innodb/sync_event/os_event_t.h>


struct buf_chunk_t;
struct hash_table_t;
struct ib_rbt_t;

/** @brief The buffer pool structure.

NOTE! The definition appears here only for other modules of this
directory (buf) to see it. Do not use from outside! */
struct buf_pool_t {
  /** @name General fields */
  /* @{ */
  BufListMutex chunks_mutex;    /*!< protects (de)allocation of chunks:
                                - changes to chunks, n_chunks are performed
                                  while holding this latch,
                                - reading buf_pool_should_madvise requires
                                  holding this latch for any buf_pool_t
                                - writing to buf_pool_should_madvise requires
                                  holding these latches for all buf_pool_t-s
                                */
  BufListMutex LRU_list_mutex;  /*!< LRU list mutex */
  BufListMutex free_list_mutex; /*!< free and withdraw list mutex */
  BufListMutex zip_free_mutex;  /*!< buddy allocator mutex */
  BufListMutex zip_hash_mutex;  /*!< zip_hash mutex */
  ib_mutex_t flush_state_mutex; /*!< Flush state protection
                              mutex */
  BufPoolZipMutex zip_mutex;    /*!< Zip mutex of this buffer
                                pool instance, protects compressed
                                only pages (of type buf_page_t, not
                                buf_block_t */
  ulint instance_no;            /*!< Array index of this buffer
                                pool instance */
  ulint curr_pool_size;         /*!< Current pool size in bytes */
  ulint LRU_old_ratio;          /*!< Reserve this much of the buffer
                                pool for "old" blocks */
#ifdef UNIV_DEBUG
  ulint buddy_n_frames; /*!< Number of frames allocated from
                        the buffer pool to the buddy system.
                        Protected by zip_hash_mutex. */
#endif
  ut_allocator<unsigned char> allocator; /*!< Allocator used for
                         allocating memory for the the "chunks"
                         member. */
  volatile ulint n_chunks;               /*!< number of buffer pool chunks */
  volatile ulint n_chunks_new; /*!< new number of buffer pool chunks */
  buf_chunk_t *chunks;         /*!< buffer pool chunks */
  buf_chunk_t *chunks_old;     /*!< old buffer pool chunks to be freed
                               after resizing buffer pool */
  ulint curr_size;             /*!< current pool size in pages */
  ulint old_size;              /*!< previous pool size in pages */
  page_no_t read_ahead_area;   /*!< size in pages of the area which
                               the read-ahead algorithms read if
                               invoked */
  hash_table_t *page_hash;     /*!< hash table of buf_page_t or
                               buf_block_t file pages,
                               buf_page_in_file() == TRUE,
                               indexed by (space_id, offset).
                               page_hash is protected by an
                               array of mutexes. */
  hash_table_t *page_hash_old; /*!< old pointer to page_hash to be
                               freed after resizing buffer pool */
  hash_table_t *zip_hash;      /*!< hash table of buf_block_t blocks
                               whose frames are allocated to the
                               zip buddy system,
                               indexed by block->frame */
  ulint n_pend_reads;          /*!< number of pending read
                               operations. Accessed atomically */
  ulint n_pend_unzip;          /*!< number of pending decompressions.
                               Accessed atomically. */

  time_t last_printout_time;
  /*!< when buf_print_io was last time
  called. Accesses not protected. */
  buf_buddy_stat_t buddy_stat[BUF_BUDDY_SIZES_MAX + 1];
  /*!< Statistics of buddy system,
  indexed by block size. Protected by
  zip_free mutex, except for the used
  field, which is also accessed
  atomically */
  buf_pool_stat_t stat;     /*!< current statistics */
  buf_pool_stat_t old_stat; /*!< old statistics */

  /* @} */

  /** @name Page flushing algorithm fields */

  /* @{ */

  BufListMutex flush_list_mutex; /*!< mutex protecting the
                                flush list access. This mutex
                                protects flush_list, flush_rbt
                                and bpage::list pointers when
                                the bpage is on flush_list. It
                                also protects writes to
                                bpage::oldest_modification and
                                flush_list_hp */
  FlushHp flush_hp;              /*!< "hazard pointer"
                                used during scan of flush_list
                                while doing flush list batch.
                                Protected by flush_list_mutex */
  UT_LIST_BASE_NODE_T(buf_page_t) flush_list;
  /*!< base node of the modified block
  list */
  ibool init_flush[BUF_FLUSH_N_TYPES];
  /*!< this is TRUE when a flush of the
  given type is being initialized.
  Protected by flush_state_mutex. */
  ulint n_flush[BUF_FLUSH_N_TYPES];
  /*!< this is the number of pending
  writes in the given flush type.
  Protected by flush_state_mutex. */
  os_event_t no_flush[BUF_FLUSH_N_TYPES];
  /*!< this is in the set state
  when there is no flush batch
  of the given type running. Protected by
  flush_state_mutex. */
  ib_rbt_t *flush_rbt;    /*!< a red-black tree is used
                          exclusively during recovery to
                          speed up insertions in the
                          flush_list. This tree contains
                          blocks in order of
                          oldest_modification LSN and is
                          kept in sync with the
                          flush_list.
                          Each member of the tree MUST
                          also be on the flush_list.
                          This tree is relevant only in
                          recovery and is set to NULL
                          once the recovery is over.
                          Protected by flush_list_mutex */
  ulint freed_page_clock; /*!< a sequence number used
                         to count the number of buffer
                         blocks removed from the end of
                         the LRU list; NOTE that this
                         counter may wrap around at 4
                         billion! A thread is allowed
                         to read this for heuristic
                         purposes without holding any
                         mutex or latch. For non-heuristic
                         purposes protected by LRU_list_mutex */
  ibool try_LRU_scan;     /*!< Set to FALSE when an LRU
                          scan for free block fails. This
                          flag is used to avoid repeated
                          scans of LRU list when we know
                          that there is no free block
                          available in the scan depth for
                          eviction. Set to TRUE whenever
                          we flush a batch from the
                          buffer pool. Accessed protected by
                          memory barriers. */

  lsn_t track_page_lsn; /* Pagge Tracking start LSN. */

  lsn_t max_lsn_io; /* Maximum LSN for which write io
                    has already started. */

  /* @} */

  /** @name LRU replacement algorithm fields */
  /* @{ */

  UT_LIST_BASE_NODE_T(buf_page_t) free;
  /*!< base node of the free
  block list */

  UT_LIST_BASE_NODE_T(buf_page_t) withdraw;
  /*!< base node of the withdraw
  block list. It is only used during
  shrinking buffer pool size, not to
  reuse the blocks will be removed.
  Protected by free_list_mutex */

  ulint withdraw_target; /*!< target length of withdraw
                         block list, when withdrawing */

  /** "hazard pointer" used during scan of LRU while doing
  LRU list batch.  Protected by buf_pool::LRU_list_mutex */
  LRUHp lru_hp;

  /** Iterator used to scan the LRU list when searching for
  replacable victim. Protected by buf_pool::LRU_list_mutex. */
  LRUItr lru_scan_itr;

  /** Iterator used to scan the LRU list when searching for
  single page flushing victim.  Protected by buf_pool::LRU_list_mutex. */
  LRUItr single_scan_itr;

  UT_LIST_BASE_NODE_T(buf_page_t) LRU;
  /*!< base node of the LRU list */

  buf_page_t *LRU_old; /*!< pointer to the about
                       LRU_old_ratio/BUF_LRU_OLD_RATIO_DIV
                       oldest blocks in the LRU list;
                       NULL if LRU length less than
                       BUF_LRU_OLD_MIN_LEN;
                       NOTE: when LRU_old != NULL, its length
                       should always equal LRU_old_len */
  ulint LRU_old_len;   /*!< length of the LRU list from
                       the block to which LRU_old points
                       onward, including that block;
                       see buf0lru.cc for the restrictions
                       on this value; 0 if LRU_old == NULL;
                       NOTE: LRU_old_len must be adjusted
                       whenever LRU_old shrinks or grows! */

  UT_LIST_BASE_NODE_T(buf_block_t) unzip_LRU;
  /*!< base node of the
  unzip_LRU list. The list is protected
  by LRU_list_mutex. */

  /* @} */
  /** @name Buddy allocator fields
  The buddy allocator is used for allocating compressed page
  frames and buf_page_t descriptors of blocks that exist
  in the buffer pool only in compressed form. */
  /* @{ */
#if defined UNIV_DEBUG || defined UNIV_BUF_DEBUG
  UT_LIST_BASE_NODE_T(buf_page_t) zip_clean;
  /*!< unmodified compressed pages */
#endif /* UNIV_DEBUG || UNIV_BUF_DEBUG */
  UT_LIST_BASE_NODE_T(buf_buddy_free_t) zip_free[BUF_BUDDY_SIZES_MAX];
  /*!< buddy free lists */

  buf_page_t *watch;
  /*!< Sentinel records for buffer
  pool watches. Scanning the array is
  protected by taking all page_hash
  latches in X. Updating or reading an
  individual watch page is protected by
  a corresponding individual page_hash
  latch. */

  /** A wrapper for buf_pool_t::allocator.alocate_large which also advices the
  OS that this chunk should not be dumped to a core file if that was requested.
  Emits a warning to the log and disables @@global.core_file if advising was
  requested but could not be performed, but still return true as the allocation
  itself succeeded.
  @param[in]	  mem_size  number of bytes to allocate
  @param[in,out]  chunk     mem and mem_pfx fields of this chunk will be updated
                            to contain information about allocated memory region
  @return true iff allocated successfully */
  bool allocate_chunk(ulonglong mem_size, buf_chunk_t *chunk);

  /** A wrapper for buf_pool_t::allocator.deallocate_large which also advices
  the OS that this chunk can be dumped to a core file.
  Emits a warning to the log and disables @@global.core_file if advising was
  requested but could not be performed.
  @param[in]  chunk   mem and mem_pfx fields of this chunk will be used to
                      locate the memory region to free */
  void deallocate_chunk(buf_chunk_t *chunk);

  /** Advices the OS that all chunks in this buffer pool instance can be dumped
  to a core file.
  Emits a warning to the log if could not succeed.
  @return true iff succeeded, false if no OS support or failed */
  bool madvise_dump();

  /** Advices the OS that all chunks in this buffer pool instance should not
  be dumped to a core file.
  Emits a warning to the log if could not succeed.
  @return true iff succeeded, false if no OS support or failed */
  bool madvise_dont_dump();

#if BUF_BUDDY_LOW > UNIV_ZIP_SIZE_MIN
#error "BUF_BUDDY_LOW > UNIV_ZIP_SIZE_MIN"
#endif
  /* @} */
};

/** Print the given buf_pool_t object.
@param[in,out]	out		the output stream
@param[in]	buf_pool	the buf_pool_t object to be printed
@return the output stream */
std::ostream &operator<<(std::ostream &out, const buf_pool_t &buf_pool);
