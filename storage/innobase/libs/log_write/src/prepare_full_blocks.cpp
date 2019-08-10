#include <innodb/log_write/prepare_full_blocks.h>

#include <innodb/log_types/log_t.h>
#include <innodb/log_block/log_block_set_flush_bit.h>
#include <innodb/log_block/log_block_set_data_len.h>
#include <innodb/log_block/log_block_set_checkpoint_no.h>
#include <innodb/log_block/log_block_store_checksum.h>
#include <innodb/log_block/log_block_set_hdr_no.h>
#include <innodb/log_block/log_block_convert_lsn_to_no.h>

namespace Log_files_write_impl {

void prepare_full_blocks(const log_t &log, byte *buffer,
                                       size_t size, lsn_t start_lsn,
                                       checkpoint_no_t checkpoint_no) {
  /* Prepare all completed blocks which are going to be written.

  Note, that completed blocks are always prepared in the log buffer,
  even if they are later copied to write_ahead buffer.

  This guarantees that finally we should have all blocks prepared
  in the log buffer (incomplete blocks will be rewritten once they
  became completed). */

  size_t buffer_offset;

  for (buffer_offset = 0; buffer_offset + OS_FILE_LOG_BLOCK_SIZE <= size;
       buffer_offset += OS_FILE_LOG_BLOCK_SIZE) {
    byte *ptr;

    ptr = buffer + buffer_offset;

    ut_a(ptr >= log.buf);

    ut_a(ptr + OS_FILE_LOG_BLOCK_SIZE <= log.buf + log.buf_size);

    log_block_set_hdr_no(
        ptr, log_block_convert_lsn_to_no(start_lsn + buffer_offset));

    log_block_set_flush_bit(ptr, buffer_offset == 0);

    log_block_set_data_len(ptr, OS_FILE_LOG_BLOCK_SIZE);

    log_block_set_checkpoint_no(ptr, checkpoint_no);

    log_block_store_checksum(ptr);
  }
}


}
