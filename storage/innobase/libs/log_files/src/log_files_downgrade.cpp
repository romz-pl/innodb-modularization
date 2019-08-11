#include <innodb/log_files/log_files_downgrade.h>

#include <innodb/disk/univ_page_size.h>
#include <innodb/io/IORequestLogWrite.h>
#include <innodb/log_block/log_block_calc_checksum_crc32.h>
#include <innodb/log_block/log_block_set_checksum.h>
#include <innodb/log_types/flags.h>
#include <innodb/log_types/log_header_format_t.h>
#include <innodb/log_types/log_t.h>
#include <innodb/machine/data.h>
#include <innodb/page/page_id_t.h>
#include <innodb/tablespace/fil_redo_io.h>

void log_files_downgrade(log_t &log) {
  ut_ad(srv_is_being_shutdown);
  ut_a(!log.checkpointer_thread_alive.load());

  const uint32_t nth_file = 0;

  byte *const buf = log.file_header_bufs[nth_file];

  const lsn_t dest_offset = nth_file * log.file_size;

  const page_no_t page_no =
      static_cast<page_no_t>(dest_offset / univ_page_size.physical());

  /* Write old version */
  mach_write_to_4(buf + LOG_HEADER_FORMAT, LOG_HEADER_FORMAT_5_7_9);

  log_block_set_checksum(buf, log_block_calc_checksum_crc32(buf));

  auto err = fil_redo_io(
      IORequestLogWrite, page_id_t{log.files_space_id, page_no}, univ_page_size,
      static_cast<ulint>(dest_offset % univ_page_size.physical()),
      OS_FILE_LOG_BLOCK_SIZE, buf);

  ut_a(err == DB_SUCCESS);
}
