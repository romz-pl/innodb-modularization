#include <innodb/log_redo/log_files_header_flush.h>

#include <innodb/io/IORequestLogWrite.h>
#include <innodb/log_types/log_t.h>
#include <innodb/log_types/LSN_PF.h>
#include <innodb/log_redo/log_files_header_fill.h>
#include <innodb/monitor/MONITOR_INC.h>
#include <innodb/tablespace/fil_redo_io.h>
#include <innodb/log_redo/flags.h>
#include <innodb/disk/univ_page_size.h>
#include <innodb/page/page_id_t.h>

void log_files_header_flush(log_t &log, uint32_t nth_file, lsn_t start_lsn) {
  ut_ad(log_writer_mutex_own(log));

  MONITOR_INC(MONITOR_LOG_NEXT_FILE);

  ut_a(nth_file < log.n_files);

  byte *buf = log.file_header_bufs[nth_file];

  log_files_header_fill(buf, start_lsn, LOG_HEADER_CREATOR_CURRENT);

  DBUG_PRINT("ib_log", ("write " LSN_PF " file " ULINTPF " header", start_lsn,
                        ulint(nth_file)));

  const auto dest_offset = nth_file * uint64_t{log.file_size};

  const auto page_no =
      static_cast<page_no_t>(dest_offset / univ_page_size.physical());

  auto err = fil_redo_io(
      IORequestLogWrite, page_id_t{log.files_space_id, page_no}, univ_page_size,
      static_cast<ulint>(dest_offset % univ_page_size.physical()),
      OS_FILE_LOG_BLOCK_SIZE, buf);

  ut_a(err == DB_SUCCESS);
}
