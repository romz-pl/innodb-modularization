#include <innodb/log_files/log_files_write_checkpoint.h>

#include <innodb/log_types/log_t.h>
#include <innodb/log_types/flags.h>
#include <innodb/log_chkp/log_checkpointer_mutex_own.h>
#include <innodb/io/srv_read_only_mode.h>
#include <innodb/log_types/log_writer_mutex_enter.h>
#include <innodb/log_types/log_writer_mutex_exit.h>
#include <innodb/machine/data.h>
#include <innodb/log_files/log_files_real_offset_for_lsn.h>
#include <innodb/log_block/log_block_calc_checksum_crc32.h>
#include <innodb/tablespace/fil_redo_io.h>
#include <innodb/io/IORequestLogWrite.h>
#include <innodb/page/page_id_t.h>
#include <innodb/log_types/LOG_SYNC_POINT.h>
#include <innodb/log_block/log_block_set_checksum.h>
#include <innodb/disk/univ_page_size.h>
#include <innodb/io/srv_unix_file_flush_method.h>
#include <innodb/tablespace/fil_flush_file_redo.h>
#include <innodb/log_types/log_update_limits.h>

void log_files_write_checkpoint(log_t &log, lsn_t next_checkpoint_lsn) {
  ut_ad(log_checkpointer_mutex_own(log));
  ut_a(!srv_read_only_mode);

  log_writer_mutex_enter(log);

  const checkpoint_no_t checkpoint_no = log.next_checkpoint_no.load();

  DBUG_PRINT("ib_log", ("checkpoint " UINT64PF " at " LSN_PF " written",
                        checkpoint_no, next_checkpoint_lsn));

  byte *buf = log.checkpoint_buf;

  memset(buf, 0x00, OS_FILE_LOG_BLOCK_SIZE);

  mach_write_to_8(buf + LOG_CHECKPOINT_NO, checkpoint_no);

  mach_write_to_8(buf + LOG_CHECKPOINT_LSN, next_checkpoint_lsn);

  const uint64_t lsn_offset =
      log_files_real_offset_for_lsn(log, next_checkpoint_lsn);

  mach_write_to_8(buf + LOG_CHECKPOINT_OFFSET, lsn_offset);

  mach_write_to_8(buf + LOG_CHECKPOINT_LOG_BUF_SIZE, log.buf_size);

  log_block_set_checksum(buf, log_block_calc_checksum_crc32(buf));

  ut_a(LOG_CHECKPOINT_1 < univ_page_size.physical());
  ut_a(LOG_CHECKPOINT_2 < univ_page_size.physical());

  /* Note: We alternate the physical place of the checkpoint info.
  See the (next_checkpoint_no & 1) below. */
  LOG_SYNC_POINT("log_before_checkpoint_write");

  auto err = fil_redo_io(
      IORequestLogWrite, page_id_t{log.files_space_id, 0}, univ_page_size,
      (checkpoint_no & 1) ? LOG_CHECKPOINT_2 : LOG_CHECKPOINT_1,
      OS_FILE_LOG_BLOCK_SIZE, buf);

  ut_a(err == DB_SUCCESS);

  LOG_SYNC_POINT("log_before_checkpoint_flush");

#ifdef _WIN32
  switch (srv_win_file_flush_method) {
    case SRV_WIN_IO_UNBUFFERED:
      break;
    case SRV_WIN_IO_NORMAL:
      fil_flush_file_redo();
      break;
  }
#else
  switch (srv_unix_file_flush_method) {
    case SRV_UNIX_O_DSYNC:
    case SRV_UNIX_NOSYNC:
      break;
    case SRV_UNIX_FSYNC:
    case SRV_UNIX_LITTLESYNC:
    case SRV_UNIX_O_DIRECT:
    case SRV_UNIX_O_DIRECT_NO_FSYNC:
      fil_flush_file_redo();
  }
#endif /* _WIN32 */

  DBUG_PRINT("ib_log", ("checkpoint info written"));

  log.next_checkpoint_no.fetch_add(1);

  LOG_SYNC_POINT("log_before_checkpoint_lsn_update");

  log.last_checkpoint_lsn.store(next_checkpoint_lsn);

  LOG_SYNC_POINT("log_before_checkpoint_limits_update");

  log_update_limits(log);

  log.dict_max_allowed_checkpoint_lsn = 0;

  log_writer_mutex_exit(log);
}
