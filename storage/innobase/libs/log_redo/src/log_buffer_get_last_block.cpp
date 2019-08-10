#include <innodb/log_redo/log_buffer_get_last_block.h>

#include <innodb/align/ut_uint64_align_down.h>
#include <innodb/log_redo/log_block_convert_lsn_to_no.h>
#include <innodb/log_redo/log_block_set_checkpoint_no.h>
#include <innodb/log_redo/log_block_set_data_len.h>
#include <innodb/log_redo/log_block_set_hdr_no.h>
#include <innodb/log_redo/log_block_store_checksum.h>
#include <innodb/log_types/log_buffer_x_lock_enter.h>
#include <innodb/log_types/log_buffer_x_lock_exit.h>
#include <innodb/log_types/log_get_lsn.h>
#include <innodb/log_types/log_t.h>

void log_buffer_get_last_block(log_t &log, lsn_t &last_lsn, byte *last_block,
                               uint32_t &block_len) {
  ut_ad(last_block != nullptr);

  /* We acquire x-lock for the log buffer to prevent:
          a) resize of the log buffer
          b) overwrite of the fragment which we are copying */

  log_buffer_x_lock_enter(log);

  /* Because we have acquired x-lock for the log buffer, current
  lsn will not advance and all users that reserved smaller lsn
  have finished writing to the log buffer. */

  last_lsn = log_get_lsn(log);

  byte *buf = log.buf;

  ut_a(buf != nullptr);

  /* Copy last block from current buffer. */

  const lsn_t block_lsn =
      ut_uint64_align_down(last_lsn, OS_FILE_LOG_BLOCK_SIZE);

  byte *src_block = buf + block_lsn % log.buf_size;

  const auto data_len = last_lsn % OS_FILE_LOG_BLOCK_SIZE;

  ut_ad(data_len >= LOG_BLOCK_HDR_SIZE);

  /* The next_checkpoint_no is protected by the x-lock too. */

  const auto checkpoint_no = log.next_checkpoint_no.load();

  std::memcpy(last_block, src_block, data_len);

  /* We have copied data from the log buffer. We can release
  the x-lock and let new writes to the buffer go. Since now,
  we work only with our local copy of the data. */

  log_buffer_x_lock_exit(log);

  std::memset(last_block + data_len, 0x00, OS_FILE_LOG_BLOCK_SIZE - data_len);

  log_block_set_hdr_no(last_block, log_block_convert_lsn_to_no(block_lsn));

  log_block_set_data_len(last_block, data_len);

  ut_ad(log_block_get_first_rec_group(last_block) <= data_len);

  log_block_set_checkpoint_no(last_block, checkpoint_no);

  log_block_store_checksum(last_block);

  block_len = OS_FILE_LOG_BLOCK_SIZE;
}

