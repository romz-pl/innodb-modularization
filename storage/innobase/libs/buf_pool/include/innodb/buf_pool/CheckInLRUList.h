#pragma once

#include <innodb/univ/univ.h>

/**********************************************************************
Let us list the consistency conditions for different control block states.

NOT_USED:	is in free list, not in LRU list, not in flush list, nor
                page hash table
READY_FOR_USE:	is not in free list, LRU list, or flush list, nor page
                hash table
MEMORY:		is not in free list, LRU list, or flush list, nor page
                hash table
FILE_PAGE:	space and offset are defined, is in page hash table
                if io_fix == BUF_IO_WRITE,
                        pool: no_flush[flush_type] is in reset state,
                        pool: n_flush[flush_type] > 0

                (1) if buf_fix_count == 0, then
                        is in LRU list, not in free list
                        is in flush list,
                                if and only if oldest_modification > 0
                        is x-locked,
                                if and only if io_fix == BUF_IO_READ
                        is s-locked,
                                if and only if io_fix == BUF_IO_WRITE

                (2) if buf_fix_count > 0, then
                        is not in LRU list, not in free list
                        is in flush list,
                                if and only if oldest_modification > 0
                        if io_fix == BUF_IO_READ,
                                is x-locked
                        if io_fix == BUF_IO_WRITE,
                                is s-locked

State transitions:

NOT_USED => READY_FOR_USE
READY_FOR_USE => MEMORY
READY_FOR_USE => FILE_PAGE
MEMORY => NOT_USED
FILE_PAGE => NOT_USED	NOTE: This transition is allowed if and only if
                                (1) buf_fix_count == 0,
                                (2) oldest_modification == 0, and
                                (3) io_fix == 0.
*/

#if defined UNIV_DEBUG || defined UNIV_BUF_DEBUG
#ifndef UNIV_HOTBACKUP
/** Functor to validate the LRU list. */
struct CheckInLRUList {
  void operator()(const buf_page_t *elem) const { ut_a(elem->in_LRU_list); }

  static void validate(const buf_pool_t *buf_pool) {
    CheckInLRUList check;
    ut_list_validate(buf_pool->LRU, check);
  }
};

#endif /* !UNIV_HOTBACKUP */
#endif /* UNIV_DEBUG || defined UNIV_BUF_DEBUG */

