#include <innodb/log_recv/recv_find_max_checkpoint.h>

#include <innodb/log_files/log_files_header_read.h>
#include <innodb/log_files/log_files_update_offsets.h>
#include <innodb/log_recv/flags.h>
#include <innodb/log_recv/recv_check_log_header_checksum.h>
#include <innodb/log_recv/recv_check_log_header_checksum.h>
#include <innodb/log_types/log_header_format_t.h>
#include <innodb/log_types/log_t.h>
#include <innodb/logger/info.h>
#include <innodb/machine/data.h>

/** Find the latest checkpoint in the log header.
@param[in,out]	log		redo log
@param[out]	max_field	LOG_CHECKPOINT_1 or LOG_CHECKPOINT_2
@return error code or DB_SUCCESS */
MY_ATTRIBUTE((warn_unused_result)) dberr_t
    recv_find_max_checkpoint(log_t &log, ulint *max_field) {
  bool found_checkpoint = false;

  *max_field = 0;

  byte *buf = log.checkpoint_buf;

  log.state = log_state_t::CORRUPTED;

  log_files_header_read(log, 0);

  /* Check the header page checksum. There was no
  checksum in the first redo log format (version 0). */
  log.format = mach_read_from_4(buf + LOG_HEADER_FORMAT);

  if (log.format != 0 && !recv_check_log_header_checksum(buf)) {
    ib::error(ER_IB_MSG_1264) << "Invalid redo log header checksum.";

    return (DB_CORRUPTION);
  }

  memcpy(log_header_creator, buf + LOG_HEADER_CREATOR,
         sizeof log_header_creator);

  log_header_creator[(sizeof log_header_creator) - 1] = 0;

  switch (log.format) {
    case 0:
      ib::error(ER_IB_MSG_1265) << "Unsupported redo log format (" << log.format
                                << "). The redo log was created"
                                << " before MySQL 5.7.9";

      return (DB_ERROR);

    case LOG_HEADER_FORMAT_5_7_9:
    case LOG_HEADER_FORMAT_8_0_1:

      ib::info(ER_IB_MSG_704, ulong{log.format});

    case LOG_HEADER_FORMAT_CURRENT:
      /* The checkpoint page format is identical upto v3. */
      break;

    default:
      ib::error(ER_IB_MSG_705, ulong{log.format}, REFMAN);

      return (DB_ERROR);
  }

  uint64_t max_no = 0;
  constexpr ulint CKP1 = LOG_CHECKPOINT_1;
  constexpr ulint CKP2 = LOG_CHECKPOINT_2;

  for (auto i = CKP1; i <= CKP2; i += CKP2 - CKP1) {
    log_files_header_read(log, static_cast<uint32_t>(i));

    if (!recv_check_log_header_checksum(buf)) {
      DBUG_PRINT("ib_log", ("invalid checkpoint, at %lu, checksum %x", i,
                            (unsigned)log_block_get_checksum(buf)));
      continue;
    }

    log.state = log_state_t::OK;

    log.current_file_lsn = mach_read_from_8(buf + LOG_CHECKPOINT_LSN);

    log.current_file_real_offset =
        mach_read_from_8(buf + LOG_CHECKPOINT_OFFSET);

    if (log.current_file_real_offset % log.file_size < LOG_FILE_HDR_SIZE) {
      log.current_file_real_offset -=
          log.current_file_real_offset % log.file_size;

      log.current_file_real_offset += LOG_FILE_HDR_SIZE;
    }

    log_files_update_offsets(log, log.current_file_lsn);

    uint64_t checkpoint_no = mach_read_from_8(buf + LOG_CHECKPOINT_NO);

    DBUG_PRINT("ib_log", ("checkpoint " UINT64PF " at " LSN_PF, checkpoint_no,
                          log.current_file_lsn));

    if (checkpoint_no >= max_no) {
      *max_field = i;
      max_no = checkpoint_no;
      found_checkpoint = true;
    }
  }

  if (!found_checkpoint) {
    /* Before 5.7.9, we could get here during database
    initialization if we created an ib_logfile0 file that
    was filled with zeroes, and were killed. After
    5.7.9, we would reject such a file already earlier,
    when checking the file header. */

    ib::error(ER_IB_MSG_706);
    return (DB_ERROR);
  }

  return (DB_SUCCESS);
}
