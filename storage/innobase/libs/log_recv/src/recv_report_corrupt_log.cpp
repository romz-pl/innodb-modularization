#include <innodb/log_recv/recv_report_corrupt_log.h>

#include <innodb/log_recv/recv_previous_parsed_rec_offset.h>
#include <innodb/log_recv/recv_previous_parsed_rec_is_multi.h>
#include <innodb/log_recv/recv_previous_parsed_rec_type.h>
#include <innodb/print/ut_print_buf.h>
#include <innodb/log_recv/recv_sys.h>
#include <innodb/logger/info.h>
#include <innodb/logger/warn.h>

#include "ha_prototypes.h"

extern ulong srv_force_recovery;

/** Prints diagnostic info of corrupt log.
@param[in]	ptr	pointer to corrupt log record
@param[in]	type	type of the log record (could be garbage)
@param[in]	space	tablespace ID (could be garbage)
@param[in]	page_no	page number (could be garbage)
@return whether processing should continue */
bool recv_report_corrupt_log(const byte *ptr, int type, space_id_t space,
                                    page_no_t page_no) {
  ib::error(ER_IB_MSG_694);

  ib::info(
      ER_IB_MSG_695, type, ulong{space}, ulong{page_no},
      ulonglong{recv_sys->recovered_lsn}, int{recv_previous_parsed_rec_type},
      ulonglong{recv_previous_parsed_rec_is_multi},
      ssize_t{ptr - recv_sys->buf}, ulonglong{recv_previous_parsed_rec_offset});

  ut_ad(ptr <= recv_sys->buf + recv_sys->len);

  const ulint limit = 100;
  const ulint before = std::min(recv_previous_parsed_rec_offset, limit);
  const ulint after = std::min(recv_sys->len - (ptr - recv_sys->buf), limit);

  ib::info(ER_IB_MSG_696, ulonglong{before}, ulonglong{after});

  ut_print_buf(
      stderr, recv_sys->buf + recv_previous_parsed_rec_offset - before,
      ptr - recv_sys->buf + before + after - recv_previous_parsed_rec_offset);
  putc('\n', stderr);

#ifndef UNIV_HOTBACKUP
  if (srv_force_recovery == 0) {
    ib::info(ER_IB_MSG_697);

    return (false);
  }
#endif /* !UNIV_HOTBACKUP */

  ib::warn(ER_IB_MSG_698, FORCE_RECOVERY_MSG);

  return (true);
}
