#include <innodb/log_redo/log_buffer_resize_low.h>

#include <innodb/log_types/log_t.h>
#include <innodb/log_types/flags.h>
#include <innodb/log_types/log_checkpointer_mutex_own.h>
#include <innodb/log_types/log_writer_mutex_own.h>
#include <innodb/align/ut_uint64_align_down.h>
#include <innodb/align/ut_uint64_align_up.h>
#include <innodb/log_redo/log_deallocate_buffer.h>
#include <innodb/log_redo/log_allocate_buffer.h>
#include <innodb/log_redo/log_calc_buf_size.h>
#include <innodb/log_redo/srv_log_buffer_size.h>
#include <innodb/logger/info.h>

bool log_buffer_resize_low(log_t &log, size_t new_size, lsn_t end_lsn) {
  ut_ad(log_checkpointer_mutex_own(log));
  ut_ad(log_writer_mutex_own(log));

  const lsn_t start_lsn =
      ut_uint64_align_down(log.write_lsn.load(), OS_FILE_LOG_BLOCK_SIZE);

  end_lsn = ut_uint64_align_up(end_lsn, OS_FILE_LOG_BLOCK_SIZE);

  if (end_lsn == start_lsn) {
    end_lsn += OS_FILE_LOG_BLOCK_SIZE;
  }

  ut_ad(end_lsn - start_lsn <= log.buf_size);

  if (end_lsn - start_lsn > new_size) {
    return (false);
  }

  /* Save the contents. */
  byte *tmp_buf = UT_NEW_ARRAY_NOKEY(byte, end_lsn - start_lsn);
  for (auto i = start_lsn; i < end_lsn; i += OS_FILE_LOG_BLOCK_SIZE) {
    std::memcpy(&tmp_buf[i - start_lsn], &log.buf[i % log.buf_size],
                OS_FILE_LOG_BLOCK_SIZE);
  }

  /* Re-allocate log buffer. */
  srv_log_buffer_size = static_cast<ulong>(new_size);
  log_deallocate_buffer(log);
  log_allocate_buffer(log);

  /* Restore the contents. */
  for (auto i = start_lsn; i < end_lsn; i += OS_FILE_LOG_BLOCK_SIZE) {
    std::memcpy(&log.buf[i % new_size], &tmp_buf[i - start_lsn],
                OS_FILE_LOG_BLOCK_SIZE);
  }
  UT_DELETE_ARRAY(tmp_buf);

  log_calc_buf_size(log);

  ut_a(srv_log_buffer_size == log.buf_size);

  ib::info(ER_IB_MSG_1260) << "srv_log_buffer_size was extended to "
                           << log.buf_size << ".";

  return (true);
}

