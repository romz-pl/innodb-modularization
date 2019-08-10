#include <innodb/log_redo/log_buffer_set_first_record_group.h>

#include <innodb/align/ut_uint64_align_down.h>
#include <innodb/log_types/LOG_SYNC_POINT.h>
#include <innodb/log_block/log_block_get_first_rec_group.h>
#include <innodb/log_block/log_block_set_first_rec_group.h>
#include <innodb/log_redo/log_buffer_ready_for_write_lsn.h>
#include <innodb/log_types/log_lsn_validate.h>
#include <innodb/log_types/Log_handle.h>
#include <innodb/log_types/log_t.h>

void log_buffer_set_first_record_group(log_t &log, const Log_handle &handle,
                                       lsn_t rec_group_end_lsn) {
  ut_ad(log.sn_lock.s_own(handle.lock_no));

  ut_a(log_lsn_validate(rec_group_end_lsn));

  const lsn_t last_block_lsn =
      ut_uint64_align_down(rec_group_end_lsn, OS_FILE_LOG_BLOCK_SIZE);

  byte *buf = log.buf;

  ut_a(buf != nullptr);

  byte *last_block_ptr = buf + (last_block_lsn % log.buf_size);

  LOG_SYNC_POINT("log_buffer_set_first_record_group_before_update");

  /* User thread needs to set proper first_rec_group value before
  link is added to recent written buffer. */
  ut_ad(log_buffer_ready_for_write_lsn(log) < rec_group_end_lsn);

  /* This also guarantees, that log buffer could not become resized
  mean while. */
  ut_a(buf + (last_block_lsn % log.buf_size) == last_block_ptr);

  /* This field is not overwritten. It is set to 0, when user thread
  crosses boundaries of consecutive log blocks. */
  ut_a(log_block_get_first_rec_group(last_block_ptr) == 0);

  log_block_set_first_rec_group(last_block_ptr,
                                rec_group_end_lsn % OS_FILE_LOG_BLOCK_SIZE);
}

