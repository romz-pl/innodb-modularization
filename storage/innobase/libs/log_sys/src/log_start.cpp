#include <innodb/log_sys/log_start.h>

#include <innodb/align/ut_uint64_align_down.h>
#include <innodb/align/ut_uint64_align_up.h>
#include <innodb/log_types/flags.h>
#include <innodb/log_block/log_block_convert_lsn_to_no.h>
#include <innodb/log_block/log_block_set_data_len.h>
#include <innodb/log_block/log_block_set_first_rec_group.h>
#include <innodb/log_block/log_block_set_flush_bit.h>
#include <innodb/log_block/log_block_set_hdr_no.h>
#include <innodb/log_buffer/log_buffer_dirty_pages_added_up_to_lsn.h>
#include <innodb/log_buffer/log_buffer_ready_for_write_lsn.h>
#include <innodb/log_files/log_files_update_offsets.h>
#include <innodb/log_types/log_sys.h>
#include <innodb/log_sn/log_translate_lsn_to_sn.h>
#include <innodb/log_types/log_update_limits.h>
#include <innodb/log_write/srv_log_write_ahead_size.h>
#include <innodb/log_types/log_t.h>

void log_start(log_t &log, checkpoint_no_t checkpoint_no, lsn_t checkpoint_lsn,
               lsn_t start_lsn) {
  ut_a(log_sys != nullptr);
  ut_a(checkpoint_lsn >= OS_FILE_LOG_BLOCK_SIZE);
  ut_a(checkpoint_lsn >= LOG_START_LSN);
  ut_a(start_lsn >= checkpoint_lsn);

  log.write_to_file_requests_total.store(0);
  log.write_to_file_requests_interval.store(0);

  log.recovered_lsn = start_lsn;
  log.last_checkpoint_lsn = checkpoint_lsn;
  log.next_checkpoint_no = checkpoint_no;
  log.available_for_checkpoint_lsn = checkpoint_lsn;

  log_update_limits(log);

  log.sn = log_translate_lsn_to_sn(log.recovered_lsn);

  if ((start_lsn + LOG_BLOCK_TRL_SIZE) % OS_FILE_LOG_BLOCK_SIZE == 0) {
    start_lsn += LOG_BLOCK_TRL_SIZE + LOG_BLOCK_HDR_SIZE;
  } else if (start_lsn % OS_FILE_LOG_BLOCK_SIZE == 0) {
    start_lsn += LOG_BLOCK_HDR_SIZE;
  }
  ut_a(start_lsn > LOG_START_LSN);

  log.recent_written.add_link(0, start_lsn);
  log.recent_written.advance_tail();
  ut_a(log_buffer_ready_for_write_lsn(log) == start_lsn);

  log.recent_closed.add_link(0, start_lsn);
  log.recent_closed.advance_tail();
  ut_a(log_buffer_dirty_pages_added_up_to_lsn(log) == start_lsn);

  log.write_lsn = start_lsn;
  log.flushed_to_disk_lsn = start_lsn;

  log_files_update_offsets(log, start_lsn);

  log.write_ahead_end_offset = ut_uint64_align_up(log.current_file_real_offset,
                                                  srv_log_write_ahead_size);

  lsn_t block_lsn;
  byte *block;

  block_lsn = ut_uint64_align_down(start_lsn, OS_FILE_LOG_BLOCK_SIZE);

  ut_a(block_lsn % log.buf_size + OS_FILE_LOG_BLOCK_SIZE <= log.buf_size);

  block = static_cast<byte *>(log.buf) + block_lsn % log.buf_size;

  log_block_set_hdr_no(block, log_block_convert_lsn_to_no(block_lsn));

  log_block_set_flush_bit(block, true);

  log_block_set_data_len(block, start_lsn - block_lsn);

  log_block_set_first_rec_group(block, start_lsn % OS_FILE_LOG_BLOCK_SIZE);

  /* Do not reorder writes above, below this line. For x86 this
  protects only from unlikely compile-time reordering. */
  std::atomic_thread_fence(std::memory_order_release);
}
