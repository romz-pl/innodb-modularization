#include <innodb/log_recv/recv_lsn_checks_on.h>

/** true if buf_page_is_corrupted() should check if the log sequence
number (FIL_PAGE_LSN) is in the future.  Initially false, and set by
recv_recovery_from_checkpoint_start(). */
bool recv_lsn_checks_on;
