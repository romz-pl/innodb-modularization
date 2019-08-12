#include <innodb/log_recv/recv_sys_var_init.h>

#include <innodb/log_recv/recv_max_page_lsn.h>
#include <innodb/log_recv/recv_n_pool_free_frames.h>
#include <innodb/log_recv/recv_previous_parsed_rec_is_multi.h>
#include <innodb/log_recv/recv_previous_parsed_rec_offset.h>
#include <innodb/log_recv/recv_previous_parsed_rec_type.h>
#include <innodb/log_recv/recv_scan_print_counter.h>
#include <innodb/log_write/recv_no_ibuf_operations.h>
#include <innodb/log_recv/recv_lsn_checks_on.h>
#include <innodb/log_recv/recv_needed_recovery.h>
#include <innodb/log_types/recv_recovery_on.h>

#ifndef UNIV_HOTBACKUP
/** Reset the state of the recovery system variables. */
void recv_sys_var_init() {
  recv_recovery_on = false;
  recv_needed_recovery = false;
  recv_lsn_checks_on = false;
  recv_no_ibuf_operations = false;
  recv_scan_print_counter = 0;
  recv_previous_parsed_rec_type = MLOG_SINGLE_REC_FLAG;
  recv_previous_parsed_rec_offset = 0;
  recv_previous_parsed_rec_is_multi = 0;
  recv_n_pool_free_frames = 256;
  recv_max_page_lsn = 0;
}
#endif /* !UNIV_HOTBACKUP */
