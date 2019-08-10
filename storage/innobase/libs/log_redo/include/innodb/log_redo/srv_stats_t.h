#pragma once

#include <innodb/univ/univ.h>

#include <innodb/counter/counter.h>
#include <innodb/log_sn/lsn_t.h>

/* Global counters used inside InnoDB. */
struct srv_stats_t {
  typedef ib_counter_t<ulint, 64> ulint_ctr_64_t;
  typedef ib_counter_t<lsn_t, 1, single_indexer_t> lsn_ctr_1_t;
  typedef ib_counter_t<ulint, 1, single_indexer_t> ulint_ctr_1_t;
  typedef ib_counter_t<lint, 1, single_indexer_t> lint_ctr_1_t;
  typedef ib_counter_t<int64_t, 1, single_indexer_t> int64_ctr_1_t;

  /** Count the amount of data written in total (in bytes) */
  ulint_ctr_1_t data_written;

  /** Number of the log write requests done */
  ulint_ctr_1_t log_write_requests;

  /** Number of physical writes to the log performed */
  ulint_ctr_1_t log_writes;

  /** Amount of data written to the log files in bytes */
  lsn_ctr_1_t os_log_written;

  /** Number of writes being done to the log files */
  lint_ctr_1_t os_log_pending_writes;

  /** We increase this counter, when we don't have enough
  space in the log buffer and have to flush it */
  ulint_ctr_1_t log_waits;

  /** Count the number of times the doublewrite buffer was flushed */
  ulint_ctr_1_t dblwr_writes;

  /** Store the number of pages that have been flushed to the
  doublewrite buffer */
  ulint_ctr_1_t dblwr_pages_written;

  /** Store the number of write requests issued */
  ulint_ctr_1_t buf_pool_write_requests;

  /** Store the number of times when we had to wait for a free page
  in the buffer pool. It happens when the buffer pool is full and we
  need to make a flush, in order to be able to read or create a page. */
  ulint_ctr_1_t buf_pool_wait_free;

  /** Count the number of pages that were written from buffer
  pool to the disk */
  ulint_ctr_1_t buf_pool_flushed;

  /** Number of buffer pool reads that led to the reading of
  a disk page */
  ulint_ctr_1_t buf_pool_reads;

  /** Number of data read in total (in bytes) */
  ulint_ctr_1_t data_read;

  /** Wait time of database locks */
  int64_ctr_1_t n_lock_wait_time;

  /** Number of database lock waits */
  ulint_ctr_1_t n_lock_wait_count;

  /** Number of threads currently waiting on database locks */
  lint_ctr_1_t n_lock_wait_current_count;

  /** Number of rows read. */
  ulint_ctr_64_t n_rows_read;

  /** Number of rows updated */
  ulint_ctr_64_t n_rows_updated;

  /** Number of rows deleted */
  ulint_ctr_64_t n_rows_deleted;

  /** Number of rows inserted */
  ulint_ctr_64_t n_rows_inserted;
};
