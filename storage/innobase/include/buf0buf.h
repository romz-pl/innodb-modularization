/*****************************************************************************

Copyright (c) 1995, 2018, Oracle and/or its affiliates. All Rights Reserved.

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

/** @file include/buf0buf.h
 The database buffer pool high-level routines

 Created 11/5/1995 Heikki Tuuri
 *******************************************************/

#ifndef buf0buf_h
#define buf0buf_h

#include <innodb/univ/univ.h>

#include <innodb/sync_rw/rw_lock_type_t.h>
#include <innodb/hash/hash_lock_s_confirm.h>
#include <innodb/hash/hash_lock_x_confirm.h>
#include <innodb/hash/hash_get_lock.h>
#include <innodb/page/page_zip_des_t.h>
#include <innodb/buf_page/buf_page_state.h>
#include <innodb/buffer/Page_fetch.h>
#include <innodb/buffer/Cache_hint.h>
#include <innodb/buf_pool/buf_pool_info_t.h>
#include <innodb/buffer/buf_pools_list_size_t.h>
#include <innodb/buf_page/flags.h>
#include <innodb/buf_page/buf_page_print_flags.h>
#include <innodb/buf_block/BPageMutex.h>
#include <innodb/buf_block/BPageLock.h>
#include <innodb/buffer/BufListMutex.h>
#include <innodb/buffer/BufPoolZipMutex.h>
#include <innodb/buf_page/buf_io_fix.h>



#include "srv0srv.h"
#include <innodb/rbt/rbt.h>

#include <ostream>

// Forward declaration
struct fil_addr_t;
class buf_page_t;
struct buf_pool_stat_t;
class FlushObserver;
struct buf_chunk_t;


/** @name Modes for buf_page_get_known_nowait */



struct buf_pool_t;

struct buf_block_t;




#ifdef UNIV_HOTBACKUP
/** first block, for --apply-log */
extern buf_block_t *back_block1;
/** second block, for page reorganize */
extern buf_block_t *back_block2;
#endif /* UNIV_HOTBACKUP */






#ifndef UNIV_HOTBACKUP
/** Creates the buffer pool.
@param[in]  total_size    Size of the total pool in bytes.
@param[in]  n_instances   Number of buffer pool instances to create.
@return DB_SUCCESS if success, DB_ERROR if not enough memory or error */
dberr_t buf_pool_init(ulint total_size, ulint n_instances);

/** Frees the buffer pool at shutdown.  This must not be invoked before
 freeing all mutexes. */
void buf_pool_free_all();

/** Determines if a block is intended to be withdrawn.
@param[in]	buf_pool	buffer pool instance
@param[in]	block		pointer to control block
@retval true	if will be withdrawn */
bool buf_block_will_withdrawn(buf_pool_t *buf_pool, const buf_block_t *block);

/** Determines if a frame is intended to be withdrawn.
@param[in]	buf_pool	buffer pool instance
@param[in]	ptr		pointer to a frame
@retval true	if will be withdrawn */
bool buf_frame_will_withdrawn(buf_pool_t *buf_pool, const byte *ptr);

/** This is the thread for resizing buffer pool. It waits for an event and
when waked up either performs a resizing and sleeps again. */
void buf_resize_thread();

/** Checks if innobase_should_madvise_buf_pool() value has changed since we've
last check and if so, then updates buf_pool_should_madvise and calls madvise
for all chunks in all srv_buf_pool_instances.
@see buf_pool_should_madvise comment for a longer explanation. */
void buf_pool_update_madvise();

/** Clears the adaptive hash index on all pages in the buffer pool. */
void buf_pool_clear_hash_index(void);

#endif /* !UNIV_HOTBACKUP */

/** Gets the smallest oldest_modification lsn among all of the earliest
added pages in flush lists. In other words - takes the last dirty page
from each flush list, and calculates minimum oldest_modification among
all of them. Does not acquire global lock for the whole process, so the
result might come from inconsistent view on flush lists.

@note Note that because of the relaxed order in each flush list, this
functions no longer returns the smallest oldest_modification among all
of the dirty pages. If you wanted to have a safe lsn, which is smaller
than every oldest_modification, you would need to use another function:
        buf_pool_get_oldest_modification_lwm().

Returns zero if there were no dirty pages (flush lists were empty).

@return minimum oldest_modification of last pages from flush lists,
        zero if flush lists were empty */
lsn_t buf_pool_get_oldest_modification_approx(void);

/** Gets a safe low watermark for oldest_modification. It's guaranteed
that there were no dirty pages with smaller oldest_modification in the
whole flush lists.

Returns zero if flush lists were empty, be careful in such case, because
taking the newest lsn is probably not a good idea. If you wanted to rely
on some lsn in such case, you would need to follow pattern:

        dpa_lsn = log_buffer_dirty_pages_added_up_to_lsn(*log_sys);

        lwm_lsn = buf_pool_get_oldest_modification_lwm();

        if (lwm_lsn == 0) lwm_lsn = dpa_lsn;

The order is important to avoid race conditions.

@remarks
It's guaranteed that the returned value will not be smaller than the
last checkpoint lsn. It's not guaranteed that the returned value is
the maximum possible. It's just the best effort for the low cost.
It basically takes result of buf_pool_get_oldest_modification_approx()
and subtracts maximum possible lag introduced by relaxed order in
flush lists (srv_log_recent_closed_size).

@return	safe low watermark for oldest_modification of dirty pages,
        or zero if flush lists were empty; if non-zero, it is then
        guaranteed not to be at block boundary (and it points to lsn
        inside data fragment of block) */
lsn_t buf_pool_get_oldest_modification_lwm(void);

#ifndef UNIV_HOTBACKUP



/** Allocates a buffer block.
 @return own: the allocated block, in state BUF_BLOCK_MEMORY */
buf_block_t *buf_block_alloc(
    buf_pool_t *buf_pool); /*!< in: buffer pool instance,
                           or NULL for round-robin selection
                           of the buffer pool */
/** Frees a buffer block which does not contain a file page. */
void buf_block_free(buf_block_t *block); /*!< in, own: block to be freed */
#endif                                   /* !UNIV_HOTBACKUP */


#ifndef UNIV_HOTBACKUP
/** NOTE! The following macros should be used instead of buf_page_get_gen,
 to improve debugging. Only values RW_S_LATCH and RW_X_LATCH are allowed
 in LA! */
#define buf_page_get(ID, SIZE, LA, MTR)                                        \
  buf_page_get_gen(ID, SIZE, LA, NULL, Page_fetch::NORMAL, __FILE__, __LINE__, \
                   MTR)
/** Use these macros to bufferfix a page with no latching. Remember not to
 read the contents of the page unless you know it is safe. Do not modify
 the contents of the page! We have separated this case, because it is
 error-prone programming not to set a latch, and it should be used
 with care. */
#define buf_page_get_with_no_latch(ID, SIZE, MTR)                     \
  buf_page_get_gen(ID, SIZE, RW_NO_LATCH, NULL, Page_fetch::NO_LATCH, \
                   __FILE__, __LINE__, MTR)

/** This is the general function used to get optimistic access to a database
page.
@param[in]      rw_latch        RW_S_LATCH, RW_X_LATCH
@param[in,out]  block           guessed block
@param[in]      modify_clock    modify clock value
@param[in]      fetch_mode      Fetch mode
@param[in]      file            file name
@param[in]      line            line where called
@param[in,out]  mtr             mini-transaction
@return true if success */
bool buf_page_optimistic_get(ulint rw_latch, buf_block_t *block,
                             uint64_t modify_clock, Page_fetch fetch_mode,
                             const char *file, ulint line, mtr_t *mtr);

/** This is used to get access to a known database page, when no waiting can be
done.
@param[in] rw_latch RW_S_LATCH or RW_X_LATCH.
@param[in] block The known page.
@param[in] hint Cache_hint::MAKE_YOUNG or Cache_hint::KEEP_OLD
@param[in] file File name from where it was called.
@param[in] line Line from where it was called.
@param[in,out] mtr Mini-transaction covering the fetch
@return true if success */
bool buf_page_get_known_nowait(ulint rw_latch, buf_block_t *block,
                               Cache_hint hint, const char *file, ulint line,
                               mtr_t *mtr);

/** Given a tablespace id and page number tries to get that page. If the
page is not in the buffer pool it is not loaded and NULL is returned.
Suitable for using when holding the lock_sys_t::mutex.
@param[in]	page_id	page id
@param[in]	file	file name
@param[in]	line	line where called
@param[in]	mtr	mini-transaction
@return pointer to a page or NULL */
const buf_block_t *buf_page_try_get_func(const page_id_t &page_id,
                                         const char *file, ulint line,
                                         mtr_t *mtr);

/** Tries to get a page.
If the page is not in the buffer pool it is not loaded. Suitable for using
when holding the lock_sys_t::mutex.
@param[in]	page_id	page identifier
@param[in]	mtr	mini-transaction
@return the page if in buffer pool, NULL if not */
#define buf_page_try_get(page_id, mtr) \
  buf_page_try_get_func((page_id), __FILE__, __LINE__, mtr);

/** Get read access to a compressed page (usually of type
FIL_PAGE_TYPE_ZBLOB or FIL_PAGE_TYPE_ZBLOB2).
The page must be released with buf_page_release_zip().
NOTE: the page is not protected by any latch.  Mutual exclusion has to
be implemented at a higher level.  In other words, all possible
accesses to a given page through this function must be protected by
the same set of mutexes or latches.
@param[in]	page_id		page id
@param[in]	page_size	page size
@return pointer to the block */
buf_page_t *buf_page_get_zip(const page_id_t &page_id,
                             const page_size_t &page_size);

/** This is the general function used to get access to a database page.
@param[in]	page_id			page id
@param[in]	page_size		page size
@param[in]	rw_latch		RW_S_LATCH, RW_X_LATCH, RW_NO_LATCH
@param[in]	guess			  guessed block or NULL
@param[in]	mode			  Fetch mode.
@param[in]	file			  file name
@param[in]	line			  line where called
@param[in]	mtr			    mini-transaction
@param[in]	dirty_with_no_latch	mark page as dirty even if page is being
                        pinned without any latch
@return pointer to the block or NULL */
buf_block_t *buf_page_get_gen(const page_id_t &page_id,
                              const page_size_t &page_size, ulint rw_latch,
                              buf_block_t *guess, Page_fetch mode,
                              const char *file, ulint line, mtr_t *mtr,
                              bool dirty_with_no_latch = false);

/** Initializes a page to the buffer buf_pool. The page is usually not read
from a file even if it cannot be found in the buffer buf_pool. This is one
of the functions which perform to a block a state transition NOT_USED =>
FILE_PAGE (the other is buf_page_get_gen). The page is latched by passed mtr.
@param[in]	page_id		page id
@param[in]	page_size	page size
@param[in]	rw_latch	RW_SX_LATCH, RW_X_LATCH
@param[in]	mtr		mini-transaction
@return pointer to the block, page bufferfixed */
buf_block_t *buf_page_create(const page_id_t &page_id,
                             const page_size_t &page_size,
                             rw_lock_type_t rw_latch, mtr_t *mtr);

#else  /* !UNIV_HOTBACKUP */

/** Inits a page to the buffer buf_pool, for use in mysqlbackup --restore.
@param[in]	page_id		page id
@param[in]	page_size	page size
@param[in,out]	block		block to init */
void meb_page_init(const page_id_t &page_id, const page_size_t &page_size,
                   buf_block_t *block);
#endif /* !UNIV_HOTBACKUP */

#ifndef UNIV_HOTBACKUP


/** Moves a page to the start of the buffer pool LRU list. This high-level
function can be used to prevent an important page from slipping out of
the buffer pool.
@param[in,out]	bpage	buffer block of a file page */
void buf_page_make_young(buf_page_t *bpage);

/** Returns TRUE if the page can be found in the buffer pool hash table.
NOTE that it is possible that the page is not yet read from disk,
though.
@param[in]	page_id	page id
@return true if found in the page hash table */
ibool buf_page_peek(const page_id_t &page_id);

#ifdef UNIV_DEBUG

/** Sets file_page_was_freed TRUE if the page is found in the buffer pool.
This function should be called when we free a file page and want the
debug version to check that it is not accessed any more unless
reallocated.
@param[in]	page_id	page id
@return control block if found in page hash table, otherwise NULL */
buf_page_t *buf_page_set_file_page_was_freed(const page_id_t &page_id);

/** Sets file_page_was_freed FALSE if the page is found in the buffer pool.
This function should be called when we free a file page and want the
debug version to check that it is not accessed any more unless
reallocated.
@param[in]	page_id	page id
@return control block if found in page hash table, otherwise NULL */
buf_page_t *buf_page_reset_file_page_was_freed(const page_id_t &page_id);

#endif /* UNIV_DEBUG */



/** Tells, for heuristics, if a block is still close enough to the MRU end of
the LRU list meaning that it is not in danger of getting evicted and also
implying that it has been accessed recently.
The page must be either buffer-fixed, either its page hash must be locked.
@param[in]	bpage	block
@return true if block is close to MRU end of LRU */
ibool buf_page_peek_if_young(const buf_page_t *bpage);

/** Recommends a move of a block to the start of the LRU list if there is
danger of dropping from the buffer pool.
NOTE: does not reserve the LRU list mutex.
@param[in]	bpage	block to make younger
@return true if should be made younger */
ibool buf_page_peek_if_too_old(const buf_page_t *bpage);



/** Increment the modify clock.
The caller must
(1) own the buf_pool->mutex and block bufferfix count has to be zero,
(2) own X or SX latch on the block->lock, or
(3) operate on a thread-private temporary table
@param[in,out]	block	buffer block */
void buf_block_modify_clock_inc(buf_block_t *block);

/** Read the modify clock.
@param[in]	block	buffer block
@return modify_clock value */
uint64_t buf_block_get_modify_clock(const buf_block_t *block);



#endif /* !UNIV_HOTBACKUP */





#ifndef UNIV_HOTBACKUP
/** Unfixes the page, unlatches the page,
removes it from page_hash and removes it from LRU.
@param[in,out]	bpage	pointer to the block */
void buf_read_page_handle_error(buf_page_t *bpage);
#else  /* !UNIV_HOTBACKUP */
#define buf_block_modify_clock_inc(block) ((void)0)
#endif /* !UNIV_HOTBACKUP */

#ifndef UNIV_HOTBACKUP


#ifdef UNIV_DEBUG
/** Finds a block in the buffer pool that points to a
given compressed page. Used only to confirm that buffer pool does not contain a
given pointer, thus protected by zip_free_mutex.
@param[in]	buf_pool	buffer pool instance
@param[in]	data		pointer to compressed page
@return buffer block pointing to the compressed page, or NULL */
buf_block_t *buf_pool_contains_zip(buf_pool_t *buf_pool, const void *data);
#endif /* UNIV_DEBUG */


#if defined UNIV_DEBUG || defined UNIV_BUF_DEBUG
/** Validates the buffer pool data structure.
 @return true */
ibool buf_validate(void);
#endif /* UNIV_DEBUG || UNIV_BUF_DEBUG */
#if defined UNIV_DEBUG_PRINT || defined UNIV_DEBUG || defined UNIV_BUF_DEBUG
/** Prints info of the buffer pool data structure. */
void buf_print(void);
#endif /* UNIV_DEBUG_PRINT || UNIV_DEBUG || UNIV_BUF_DEBUG */
#endif /* !UNIV_HOTBACKUP */



/** Prints a page to stderr.
@param[in]	read_buf	a database page
@param[in]	page_size	page size
@param[in]	flags		0 or BUF_PAGE_PRINT_NO_CRASH or
BUF_PAGE_PRINT_NO_FULL */
void buf_page_print(const byte *read_buf, const page_size_t &page_size,
                    ulint flags);

/** Decompress a block.
 @return true if successful */
ibool buf_zip_decompress(buf_block_t *block, /*!< in/out: block */
                         ibool check); /*!< in: TRUE=verify the page checksum */
#ifndef UNIV_HOTBACKUP
#ifdef UNIV_DEBUG
/** Returns the number of latched pages in the buffer pool.
 @return number of latched pages */
ulint buf_get_latched_pages_number(void);
#endif /* UNIV_DEBUG */
/** Returns the number of pending buf pool read ios.
 @return number of pending read I/O operations */
ulint buf_get_n_pending_read_ios(void);
/** Prints info of the buffer i/o. */
void buf_print_io(FILE *file); /*!< in: file where to print */
/** Collect buffer pool stats information for a buffer pool. Also
 record aggregated stats if there are more than one buffer pool
 in the server */
void buf_stats_get_pool_info(
    buf_pool_t *buf_pool,            /*!< in: buffer pool */
    ulint pool_id,                   /*!< in: buffer pool ID */
    buf_pool_info_t *all_pool_info); /*!< in/out: buffer pool info
                                     to fill */
/** Return the ratio in percents of modified pages in the buffer pool /
database pages in the buffer pool.
@return modified page percentage ratio */
double buf_get_modified_ratio_pct(void);
/** Refresh the statistics used to print per-second averages. */
void buf_refresh_io_stats_all();

/** Assert that all file pages in the buffer are in a replaceable state. */
void buf_must_be_all_freed(void);

/** Checks that there currently are no pending i/o-operations for the buffer
pool.
@return number of pending i/o */
ulint buf_pool_check_no_pending_io(void);

/** Invalidates the file pages in the buffer pool when an archive recovery is
 completed. All the file pages buffered must be in a replaceable state when
 this function is called: not latched and not modified. */
void buf_pool_invalidate(void);
#endif /* !UNIV_HOTBACKUP */

/*========================================================================
--------------------------- LOWER LEVEL ROUTINES -------------------------
=========================================================================*/

#ifdef UNIV_DEBUG
/** Adds latch level info for the rw-lock protecting the buffer frame. This
should be called in the debug version after a successful latching of a page if
we know the latching order level of the acquired latch.
@param[in]	block	buffer page where we have acquired latch
@param[in]	level	latching order level */
void buf_block_dbg_add_level(buf_block_t *block, latch_level_t level);
#else                                         /* UNIV_DEBUG */
#define buf_block_dbg_add_level(block, level) /* nothing */
#endif                                        /* UNIV_DEBUG */



/** Gets the compressed page descriptor corresponding to an uncompressed page
 if applicable. */
#define buf_block_get_page_zip(block) \
  ((block)->page.zip.data ? &(block)->page.zip : NULL)

/** Get a buffer block from an adaptive hash index pointer.
This function does not return if the block is not identified.
@param[in]	ptr	pointer to within a page frame
@return pointer to block, never NULL */
buf_block_t *buf_block_from_ahi(const byte *ptr);

#ifndef UNIV_HOTBACKUP
/** Find out if a pointer belongs to a buf_block_t. It can be a pointer to
 the buf_block_t itself or a member of it
 @return true if ptr belongs to a buf_block_t struct */
ibool buf_pointer_is_block_field(const void *ptr); /*!< in: pointer not
                                                   dereferenced */
/** Find out if a pointer corresponds to a buf_block_t::mutex.
@param m in: mutex candidate
@return true if m is a buf_block_t::mutex */
#define buf_pool_is_block_mutex(m) buf_pointer_is_block_field((const void *)(m))
/** Find out if a pointer corresponds to a buf_block_t::lock.
@param l in: rw-lock candidate
@return true if l is a buf_block_t::lock */
#define buf_pool_is_block_lock(l) buf_pointer_is_block_field((const void *)(l))

/** Inits a page for read to the buffer buf_pool. If the page is
(1) already in buf_pool, or
(2) if we specify to read only ibuf pages and the page is not an ibuf page, or
(3) if the space is deleted or being deleted,
then this function does nothing.
Sets the io_fix flag to BUF_IO_READ and sets a non-recursive exclusive lock
on the buffer frame. The io-handler must take care that the flag is cleared
and the lock released later.
@param[out]	err			DB_SUCCESS or DB_TABLESPACE_DELETED
@param[in]	mode			BUF_READ_IBUF_PAGES_ONLY, ...
@param[in]	page_id			page id
@param[in]	page_size		page size
@param[in]	unzip			TRUE=request uncompressed page
@return pointer to the block or NULL */
buf_page_t *buf_page_init_for_read(dberr_t *err, ulint mode,
                                   const page_id_t &page_id,
                                   const page_size_t &page_size, ibool unzip);

/** Completes an asynchronous read or write request of a file page to or from
the buffer pool.
@param[in]	bpage	pointer to the block in question
@param[in]	evict	whether or not to evict the page from LRU list
@return true if successful */
bool buf_page_io_complete(buf_page_t *bpage, bool evict = false);


/** Returns the control block of a file page, NULL if not found.
@param[in]	buf_pool	buffer pool instance
@param[in]	page_id		page id
@return block, NULL if not found */
buf_page_t *buf_page_hash_get_low(buf_pool_t *buf_pool,
                                  const page_id_t &page_id);

/** Returns the control block of a file page, NULL if not found.
If the block is found and lock is not NULL then the appropriate
page_hash lock is acquired in the specified lock mode. Otherwise,
mode value is ignored. It is up to the caller to release the
lock. If the block is found and the lock is NULL then the page_hash
lock is released by this function.
@param[in]	buf_pool	buffer pool instance
@param[in]	page_id		page id
@param[in,out]	lock		lock of the page hash acquired if bpage is
found, NULL otherwise. If NULL is passed then the hash_lock is released by
this function.
@param[in]	lock_mode	RW_LOCK_X or RW_LOCK_S. Ignored if
lock == NULL
@param[in]	watch		if true, return watch sentinel also.
@return pointer to the bpage or NULL; if NULL, lock is also NULL or
a watch sentinel. */
buf_page_t *buf_page_hash_get_locked(buf_pool_t *buf_pool,
                                     const page_id_t &page_id, rw_lock_t **lock,
                                     ulint lock_mode, bool watch = false);

/** Returns the control block of a file page, NULL if not found.
If the block is found and lock is not NULL then the appropriate
page_hash lock is acquired in the specified lock mode. Otherwise,
mode value is ignored. It is up to the caller to release the
lock. If the block is found and the lock is NULL then the page_hash
lock is released by this function.
@param[in]	buf_pool	buffer pool instance
@param[in]	page_id		page id
@param[in,out]	lock		lock of the page hash acquired if bpage is
found, NULL otherwise. If NULL is passed then the hash_lock is released by
this function.
@param[in]	lock_mode	RW_LOCK_X or RW_LOCK_S. Ignored if
lock == NULL
@return pointer to the block or NULL; if NULL, lock is also NULL. */
buf_block_t *buf_block_hash_get_locked(buf_pool_t *buf_pool,
                                       const page_id_t &page_id,
                                       rw_lock_t **lock, ulint lock_mode);

/* There are four different ways we can try to get a bpage or block
from the page hash:
1) Caller already holds the appropriate page hash lock: in the case call
buf_page_hash_get_low() function.
2) Caller wants to hold page hash lock in x-mode
3) Caller wants to hold page hash lock in s-mode
4) Caller doesn't want to hold page hash lock */
#define buf_page_hash_get_s_locked(b, page_id, l) \
  buf_page_hash_get_locked(b, page_id, l, RW_LOCK_S)
#define buf_page_hash_get_x_locked(b, page_id, l) \
  buf_page_hash_get_locked(b, page_id, l, RW_LOCK_X)
#define buf_page_hash_get(b, page_id) \
  buf_page_hash_get_locked(b, page_id, NULL, 0)
#define buf_page_get_also_watch(b, page_id) \
  buf_page_hash_get_locked(b, page_id, NULL, 0, true)

#define buf_block_hash_get_s_locked(b, page_id, l) \
  buf_block_hash_get_locked(b, page_id, l, RW_LOCK_S)
#define buf_block_hash_get_x_locked(b, page_id, l) \
  buf_block_hash_get_locked(b, page_id, l, RW_LOCK_X)
#define buf_block_hash_get(b, page_id) \
  buf_block_hash_get_locked(b, page_id, NULL, 0)

/** Gets the current length of the free list of buffer blocks.
 @return length of the free list */
ulint buf_get_free_list_len(void);

/** Determine if a block is a sentinel for a buffer pool watch.
 @return true if a sentinel for a buffer pool watch, false if not */
ibool buf_pool_watch_is_sentinel(
    const buf_pool_t *buf_pool, /*!< buffer pool instance */
    const buf_page_t *bpage)    /*!< in: block */
    MY_ATTRIBUTE((warn_unused_result));

/** Stop watching if the page has been read in.
buf_pool_watch_set(space,offset) must have returned NULL before.
@param[in]	page_id	page id */
void buf_pool_watch_unset(const page_id_t &page_id);

/** Check if the page has been read in.
This may only be called after buf_pool_watch_set(space,offset)
has returned NULL and before invoking buf_pool_watch_unset(space,offset).
@param[in]	page_id	page id
@return false if the given page was not read in, true if it was */
ibool buf_pool_watch_occurred(const page_id_t &page_id)
    MY_ATTRIBUTE((warn_unused_result));


#include <innodb/buffer/buf_get_total_stat.h>
#include <innodb/buffer/buf_get_total_list_size_in_bytes.h>
#include <innodb/buffer/buf_get_total_list_len.h>


/** Calculate the checksum of a page from compressed table and update the
page.
@param[in,out]  page              page to update
@param[in]      size              compressed page size
@param[in]      lsn               LSN to stamp on the page
@param[in]      skip_lsn_check    true to skip check for lsn (in DEBUG) */
void buf_flush_update_zip_checksum(buf_frame_t *page, ulint size, lsn_t lsn,
                                   bool skip_lsn_check);

#endif /* !UNIV_HOTBACKUP */


/** Number of bits used for buffer page states. */
#define BUF_PAGE_STATE_BITS 3




/** Compute the hash fold value for blocks in buf_pool->zip_hash. */
/* @{ */
#define BUF_POOL_ZIP_FOLD_PTR(ptr) ((ulint)(ptr) / UNIV_PAGE_SIZE)
#define BUF_POOL_ZIP_FOLD(b) BUF_POOL_ZIP_FOLD_PTR((b)->frame)
#define BUF_POOL_ZIP_FOLD_BPAGE(b) BUF_POOL_ZIP_FOLD((buf_block_t *)(b))
/* @} */



/** @name Accessors for buffer pool mutexes
Use these instead of accessing buffer pool mutexes directly. */
/* @{ */

#ifndef UNIV_HOTBACKUP

/** Get appropriate page_hash_lock. */
#define buf_page_hash_lock_get(buf_pool, page_id) \
  hash_get_lock((buf_pool)->page_hash, (page_id).fold())

/** If not appropriate page_hash_lock, relock until appropriate. */
#define buf_page_hash_lock_s_confirm(hash_lock, buf_pool, page_id) \
  hash_lock_s_confirm(hash_lock, (buf_pool)->page_hash, (page_id).fold())

#define buf_page_hash_lock_x_confirm(hash_lock, buf_pool, page_id) \
  hash_lock_x_confirm(hash_lock, (buf_pool)->page_hash, (page_id).fold())
#endif /* !UNIV_HOTBACKUP */

#if defined(UNIV_DEBUG) && !defined(UNIV_HOTBACKUP)
/** Test if page_hash lock is held in s-mode. */
#define buf_page_hash_lock_held_s(buf_pool, bpage) \
  rw_lock_own(buf_page_hash_lock_get((buf_pool), (bpage)->id), RW_LOCK_S)

/** Test if page_hash lock is held in x-mode. */
#define buf_page_hash_lock_held_x(buf_pool, bpage) \
  rw_lock_own(buf_page_hash_lock_get((buf_pool), (bpage)->id), RW_LOCK_X)

/** Test if page_hash lock is held in x or s-mode. */
#define buf_page_hash_lock_held_s_or_x(buf_pool, bpage) \
  (buf_page_hash_lock_held_s((buf_pool), (bpage)) ||    \
   buf_page_hash_lock_held_x((buf_pool), (bpage)))

#define buf_block_hash_lock_held_s(buf_pool, block) \
  buf_page_hash_lock_held_s((buf_pool), &(block)->page)

#define buf_block_hash_lock_held_x(buf_pool, block) \
  buf_page_hash_lock_held_x((buf_pool), &(block)->page)

#define buf_block_hash_lock_held_s_or_x(buf_pool, block) \
  buf_page_hash_lock_held_s_or_x((buf_pool), &(block)->page)
#else /* UNIV_DEBUG && !UNIV_HOTBACKUP */
#define buf_page_hash_lock_held_s(b, p) (TRUE)
#define buf_page_hash_lock_held_x(b, p) (TRUE)
#define buf_page_hash_lock_held_s_or_x(b, p) (TRUE)
#define buf_block_hash_lock_held_s(b, p) (TRUE)
#define buf_block_hash_lock_held_x(b, p) (TRUE)
#define buf_block_hash_lock_held_s_or_x(b, p) (TRUE)
#endif /* UNIV_DEBUG && !UNIV_HOTBACKUP */

/* @} */



#endif /* !buf0buf_h */
