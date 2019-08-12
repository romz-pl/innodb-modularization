#include <innodb/log_recv/recv_log_recover_pre_8_0_4.h>

#include <innodb/disk/page_no_t.h>
#include <innodb/log_files/log_files_real_offset_for_lsn.h>
#include <innodb/disk/univ_page_size.h>
#include <innodb/tablespace/fil_redo_io.h>
#include <innodb/io/IORequestLogRead.h>
#include <innodb/page/page_id_t.h>
#include <innodb/log_block/log_block_get_data_len.h>
#include <innodb/log_sys/log_start.h>
#include <innodb/align/ut_uint64_align_down.h>
#include <innodb/log_types/log_t.h>
#include <innodb/log_block/log_block_calc_checksum.h>
#include <innodb/log_block/log_block_get_checksum.h>
#include <innodb/log_recv/flags.h>
#include <innodb/log_files/srv_log_file_size.h>
#include <innodb/log_recv/recv_sys.h>

/** Determine if a redo log from a version before MySQL 8.0.3 is clean.
@param[in,out]	log		redo log
@param[in]	checkpoint_no	checkpoint number
@param[in]	checkpoint_lsn	checkpoint LSN
@return error code
@retval DB_SUCCESS	if the redo log is clean
@retval DB_ERROR	if the redo log is corrupted or dirty */
dberr_t recv_log_recover_pre_8_0_4(log_t &log,
                                          checkpoint_no_t checkpoint_no,
                                          lsn_t checkpoint_lsn) {
  lsn_t source_offset;
  lsn_t block_lsn;
  page_no_t page_no;
  byte *buf;

  source_offset = log_files_real_offset_for_lsn(log, checkpoint_lsn);

  block_lsn = ut_uint64_align_down(checkpoint_lsn, OS_FILE_LOG_BLOCK_SIZE);

  page_no = (page_no_t)(source_offset / univ_page_size.physical());

  buf = log.buf + block_lsn % log.buf_size;

  static const char *NO_UPGRADE_RECOVERY_MSG =
      "Upgrade after a crash is not supported."
      " This redo log was created with ";

  static const char *NO_UPGRADE_RTFM_MSG =
      ". Please follow the instructions at " REFMAN "upgrading.html";

  dberr_t err;

  err = fil_redo_io(IORequestLogRead, page_id_t(log.files_space_id, page_no),
                    univ_page_size,
                    (ulint)((source_offset & ~(OS_FILE_LOG_BLOCK_SIZE - 1)) %
                            univ_page_size.physical()),
                    OS_FILE_LOG_BLOCK_SIZE, buf);

  ut_a(err == DB_SUCCESS);

  if (log_block_calc_checksum(buf) != log_block_get_checksum(buf)) {
    ib::error(ER_IB_MSG_700)
        << NO_UPGRADE_RECOVERY_MSG << log_header_creator
        << ", and it appears corrupted" << NO_UPGRADE_RTFM_MSG;

    return (DB_CORRUPTION);
  }

  /* On a clean shutdown, the redo log will be logically empty
  after the checkpoint LSN. */

  if (log_block_get_data_len(buf) !=
      (source_offset & (OS_FILE_LOG_BLOCK_SIZE - 1))) {
    ib::error(ER_IB_MSG_701)
        << NO_UPGRADE_RECOVERY_MSG << log_header_creator << NO_UPGRADE_RTFM_MSG;

    return (DB_ERROR);
  }

  /* Mark the redo log for upgrading. */
  srv_log_file_size = 0;

  recv_sys->parse_start_lsn = checkpoint_lsn;
  recv_sys->bytes_to_ignore_before_checkpoint = 0;
  recv_sys->recovered_lsn = checkpoint_lsn;
  recv_sys->checkpoint_lsn = checkpoint_lsn;
  recv_sys->scanned_lsn = checkpoint_lsn;

  log_start(log, checkpoint_no + 1, checkpoint_lsn, checkpoint_lsn);

  return (DB_SUCCESS);
}
