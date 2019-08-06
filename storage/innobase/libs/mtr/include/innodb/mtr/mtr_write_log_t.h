#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/mtr/mtr_buf_t.h>
#include <innodb/tablespace/lsn_t.h>

lsn_t log_buffer_write(log_t &log, const Log_handle &handle, const byte *str,
                       size_t str_len, lsn_t start_lsn);

void log_buffer_set_first_record_group(log_t &log, const Log_handle &handle,
                                       lsn_t rec_group_end_lsn);

/** Write the block contents to the REDO log */
struct mtr_write_log_t {
  /** Append a block to the redo log buffer.
  @return whether the appending should continue */
  bool operator()(const mtr_buf_t::block_t *block) {
    lsn_t start_lsn;
    lsn_t end_lsn;

    ut_ad(block != nullptr);

    if (block->used() == 0) {
      return (true);
    }

    start_lsn = m_lsn;

    end_lsn = log_buffer_write(*log_sys, m_handle, block->begin(),
                               block->used(), start_lsn);

    ut_a(end_lsn % OS_FILE_LOG_BLOCK_SIZE <
         OS_FILE_LOG_BLOCK_SIZE - LOG_BLOCK_TRL_SIZE);

    m_left_to_write -= block->used();

    if (m_left_to_write == 0
        /* This write was up to the end of record group,
        the last record in group has been written.

        Therefore next group of records starts at m_lsn.
        We need to find out, if the next group is the first group,
        that starts in this log block.

        In such case we need to set first_rec_group.

        Now, we could have two cases:
        1. This group of log records has started in previous block
           to block containing m_lsn.
        2. This group of log records has started in the same block
           as block containing m_lsn.

        Only in case 1), the next group of records is the first group
        of log records in block containing m_lsn. */
        && m_handle.start_lsn / OS_FILE_LOG_BLOCK_SIZE !=
               end_lsn / OS_FILE_LOG_BLOCK_SIZE) {
      log_buffer_set_first_record_group(*log_sys, m_handle, end_lsn);
    }

    log_buffer_write_completed(*log_sys, m_handle, start_lsn, end_lsn);

    m_lsn = end_lsn;

    return (true);
  }

  Log_handle m_handle;
  lsn_t m_lsn;
  ulint m_left_to_write;
};
#endif /* !UNIV_HOTBACKUP */
