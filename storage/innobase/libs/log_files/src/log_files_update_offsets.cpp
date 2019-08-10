#include <innodb/log_files/log_files_update_offsets.h>

#include <innodb/log_types/log_t.h>
#include <innodb/log_files/log_files_real_offset_for_lsn.h>
#include <innodb/align/ut_uint64_align_down.h>

#ifndef UNIV_HOTBACKUP

void log_files_update_offsets(log_t &log, lsn_t lsn) {
  ut_ad(log_writer_mutex_own(log));
  ut_a(log.file_size > 0);
  ut_a(log.n_files > 0);

  lsn = ut_uint64_align_down(lsn, OS_FILE_LOG_BLOCK_SIZE);

  log.current_file_real_offset = log_files_real_offset_for_lsn(log, lsn);

  /* Real offsets never enter headers of files when calculated
  for some LSN / size offset. */
  ut_a(log.current_file_real_offset % log.file_size >= LOG_FILE_HDR_SIZE);

  log.current_file_lsn = lsn;

  log.current_file_end_offset = log.current_file_real_offset -
                                log.current_file_real_offset % log.file_size +
                                log.file_size;

  ut_a(log.current_file_end_offset % log.file_size == 0);
}


#endif
