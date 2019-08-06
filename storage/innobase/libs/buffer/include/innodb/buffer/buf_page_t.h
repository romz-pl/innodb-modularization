#pragma once

#include <innodb/univ/univ.h>

#include <innodb/buffer/buf_io_fix.h>
#include <innodb/buffer/buf_page_state.h>
#include <innodb/log_types/lsn_t.h>
#include <innodb/buffer/macros.h>
#include <innodb/page/page_id_t.h>
#include <innodb/lst/lst.h>
#include <innodb/disk/page_size_t.h>
#include <innodb/page/page_zip_des_t.h>

class FlushObserver;

/** The common buffer control block structure
for compressed and uncompressed frames */
class buf_page_t {
 public:
  /** @name General fields
  None of these bit-fields must be modified without holding
  buf_page_get_mutex() [buf_block_t::mutex or
  buf_pool->zip_mutex], since they can be stored in the same
  machine word.  */
  /* @{ */

  /** Page id. */
  page_id_t id;

  /** Page size. */
  page_size_t size;

  /** Count of how manyfold this block is currently bufferfixed. */
  uint32_t buf_fix_count;

  /** type of pending I/O operation. */
  buf_io_fix io_fix;

  /** Block state. @see buf_page_in_file */
  buf_page_state state;

  /** if this block is currently being flushed to disk, this tells
  the flush_type.  @see buf_flush_t */
  unsigned flush_type : 2;

  /** index number of the buffer pool that this block belongs to */
  unsigned buf_pool_index : 6;

  static_assert(MAX_BUFFER_POOLS <= 64,
                "MAX_BUFFER_POOLS > 64; redefine buf_pool_index");

  /* @} */
  page_zip_des_t zip; /*!< compressed page; zip.data
                      (but not the data it points to) is
                      protected by buf_pool->zip_mutex;
                      state == BUF_BLOCK_ZIP_PAGE and
                      zip.data == NULL means an active
                      buf_pool->watch */
#ifndef UNIV_HOTBACKUP
  buf_page_t *hash; /*!< node used in chaining to
                    buf_pool->page_hash or
                    buf_pool->zip_hash */
#endif              /* !UNIV_HOTBACKUP */
#ifdef UNIV_DEBUG
  ibool in_page_hash; /*!< TRUE if in buf_pool->page_hash */
  ibool in_zip_hash;  /*!< TRUE if in buf_pool->zip_hash */
#endif                /* UNIV_DEBUG */

  /** @name Page flushing fields
  All these are protected by buf_pool->mutex. */
  /* @{ */

  UT_LIST_NODE_T(buf_page_t) list;
  /*!< based on state, this is a
  list node, protected by the
  corresponding list mutex, in one of the
  following lists in buf_pool:

  - BUF_BLOCK_NOT_USED:	free, withdraw
  - BUF_BLOCK_FILE_PAGE:	flush_list
  - BUF_BLOCK_ZIP_DIRTY:	flush_list
  - BUF_BLOCK_ZIP_PAGE:	zip_clean

  The node pointers are protected by the
  corresponding list mutex.

  The contents of the list node
  is undefined if !in_flush_list
  && state == BUF_BLOCK_FILE_PAGE,
  or if state is one of
  BUF_BLOCK_MEMORY,
  BUF_BLOCK_REMOVE_HASH or
  BUF_BLOCK_READY_IN_USE. */

#ifdef UNIV_DEBUG
  ibool in_flush_list; /*!< TRUE if in buf_pool->flush_list;
                       when buf_pool->flush_list_mutex is
                       free, the following should hold:
                       in_flush_list
                       == (state == BUF_BLOCK_FILE_PAGE
                           || state == BUF_BLOCK_ZIP_DIRTY)
                       Writes to this field must be
                       covered by both block->mutex
                       and buf_pool->flush_list_mutex. Hence
                       reads can happen while holding
                       any one of the two mutexes */
  ibool in_free_list;  /*!< TRUE if in buf_pool->free; when
                       buf_pool->free_list_mutex is free, the
                       following should hold: in_free_list
                       == (state == BUF_BLOCK_NOT_USED) */
#endif                 /* UNIV_DEBUG */

  FlushObserver *flush_observer; /*!< flush observer */

  lsn_t newest_modification;
  /*!< log sequence number of
  the youngest modification to
  this block, zero if not
  modified. Protected by block
  mutex */
  lsn_t oldest_modification;
  /*!< log sequence number of
  the START of the log entry
  written of the oldest
  modification to this block
  which has not yet been flushed
  on disk; zero if all
  modifications are on disk.
  Writes to this field must be
  covered by both block->mutex
  and buf_pool->flush_list_mutex. Hence
  reads can happen while holding
  any one of the two mutexes */
  /* @} */
  /** @name LRU replacement algorithm fields
  These fields are protected by both buf_pool->LRU_list_mutex and the
  block mutex. */
  /* @{ */

  UT_LIST_NODE_T(buf_page_t) LRU;
  /*!< node of the LRU list */
#ifdef UNIV_DEBUG
  ibool in_LRU_list; /*!< TRUE if the page is in
                     the LRU list; used in
                     debugging */
#endif               /* UNIV_DEBUG */
#ifndef UNIV_HOTBACKUP
  unsigned old : 1;               /*!< TRUE if the block is in the old
                                  blocks in buf_pool->LRU_old */
  unsigned freed_page_clock : 31; /*!< the value of
                              buf_pool->freed_page_clock
                              when this block was the last
                              time put to the head of the
                              LRU list; a thread is allowed
                              to read this for heuristic
                              purposes without holding any
                              mutex or latch */
  /* @} */
  unsigned access_time; /*!< time of first access, or
                        0 if the block was never accessed
                        in the buffer pool. Protected by
                        block mutex */
#ifdef UNIV_DEBUG
  ibool file_page_was_freed;
  /*!< this is set to TRUE when
  fsp frees a page in buffer pool;
  protected by buf_pool->zip_mutex
  or buf_block_t::mutex. */
#endif /* UNIV_DEBUG */
#endif /* !UNIV_HOTBACKUP */
};
