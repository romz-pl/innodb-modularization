#include <innodb/log_write/copy_to_write_ahead_buffer.h>

#include <innodb/align/ut_uint64_align_down.h>
#include <innodb/log_redo/log_block_convert_lsn_to_no.h>
#include <innodb/log_redo/log_block_get_first_rec_group.h>
#include <innodb/log_redo/log_block_set_checkpoint_no.h>
#include <innodb/log_redo/log_block_set_data_len.h>
#include <innodb/log_redo/log_block_set_first_rec_group.h>
#include <innodb/log_redo/log_block_set_flush_bit.h>
#include <innodb/log_redo/log_block_set_hdr_no.h>
#include <innodb/log_redo/log_block_store_checksum.h>
#include <innodb/log_redo/srv_log_write_ahead_size.h>
#include <innodb/log_redo/LOG_SYNC_POINT.h>
#include <innodb/log_types/log_t.h>
#include <innodb/log_types/flags.h>

namespace Log_files_write_impl {

void copy_to_write_ahead_buffer(log_t &log, const byte *buffer,
                                              size_t &size, lsn_t start_lsn,
                                              checkpoint_no_t checkpoint_no) {
  ut_a(size <= srv_log_write_ahead_size);

  ut_a(buffer >= log.buf);
  ut_a(buffer + size <= log.buf + log.buf_size);

  byte *write_buf = log.write_ahead_buf;

  LOG_SYNC_POINT("log_writer_before_copy_to_write_ahead_buffer");

  std::memcpy(write_buf, buffer, size);

  size_t completed_blocks_size;
  byte *incomplete_block;
  size_t incomplete_size;

  completed_blocks_size = ut_uint64_align_down(size, OS_FILE_LOG_BLOCK_SIZE);

  incomplete_block = write_buf + completed_blocks_size;

  incomplete_size = size % OS_FILE_LOG_BLOCK_SIZE;

  ut_a(incomplete_block + incomplete_size <=
       write_buf + srv_log_write_ahead_size);

  if (incomplete_size != 0) {
    /* Prepare the incomplete (last) block. */
    ut_a(incomplete_size >= LOG_BLOCK_HDR_SIZE);

    log_block_set_hdr_no(
        incomplete_block,
        log_block_convert_lsn_to_no(start_lsn + completed_blocks_size));

    log_block_set_flush_bit(incomplete_block, completed_blocks_size == 0);

    log_block_set_data_len(incomplete_block, incomplete_size);

    if (log_block_get_first_rec_group(incomplete_block) > incomplete_size) {
      log_block_set_first_rec_group(incomplete_block, 0);
    }

    log_block_set_checkpoint_no(incomplete_block, checkpoint_no);

    std::memset(incomplete_block + incomplete_size, 0x00,
                OS_FILE_LOG_BLOCK_SIZE - incomplete_size);

    log_block_store_checksum(incomplete_block);

    size = completed_blocks_size + OS_FILE_LOG_BLOCK_SIZE;
  }

  /* Since now, size is about completed blocks always. */
  ut_a(size % OS_FILE_LOG_BLOCK_SIZE == 0);
}



}
