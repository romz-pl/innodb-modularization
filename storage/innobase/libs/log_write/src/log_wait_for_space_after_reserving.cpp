#include <innodb/log_write/log_wait_for_space_after_reserving.h>

#include <innodb/align/ut_uint64_align_up.h>
#include <innodb/log_types/LOG_SYNC_POINT.h>
#include <innodb/log_redo/log_buffer_resize_low.h>
#include <innodb/log_types/log_checkpointer_mutex_enter.h>
#include <innodb/log_types/log_checkpointer_mutex_exit.h>
#include <innodb/log_redo/log_translate_lsn_to_sn.h>
#include <innodb/log_sn/log_translate_sn_to_lsn.h>
#include <innodb/log_types/log_writer_mutex_enter.h>
#include <innodb/log_types/log_writer_mutex_exit.h>
#include <innodb/log_types/Log_handle.h>
#include <innodb/log_types/log_t.h>
#include <innodb/log_write/log_wait_for_space_in_log_buf.h>
#include <innodb/log_write/log_write_up_to.h>
#include <innodb/logger/info.h>

void log_wait_for_space_after_reserving(log_t &log,
                                               const Log_handle &handle) {
  ut_ad(log.sn_lock.s_own(handle.lock_no));

  const sn_t start_sn = log_translate_lsn_to_sn(handle.start_lsn);

  const sn_t end_sn = log_translate_lsn_to_sn(handle.end_lsn);

  const sn_t len = end_sn - start_sn;

  /* If we had not allowed to resize log buffer, it would have
  been sufficient here to simply call:
          - log_wait_for_space_in_log_buf(log, end_sn).

  However we do allow, and we need to handle the possible race
  condition, when user tries to set very small log buffer size
  and other threads try to write large groups of log records.

  Note that since this point, log.buf_size_sn may only be
  increased from our point of view. That's because:

          1. Other threads doing mtr_commit will only try to
             increase the size (if needed).

          2. If user wanted to manually resize the log buffer,
             he needs to obtain x-lock for the redo log, but
             we keep s-lock. */

  log_wait_for_space_in_log_buf(log, start_sn);

  /* Now start_sn fits the log buffer or is at the boundary.
  Therefore all previous reservations (for smaller LSN), fit
  the log buffer [1].

  We check if len > log.buf_size_sn. If that's the case, our
  range start_sn..end_sn will cover more than size of the log
  buffer and we need to extend the size. Note that users that
  reserved smaller LSN will not try to extend because of [1].
  Users that reserved larger LSN, will not have their start_sn
  in the log buffer, because our end_sn already does not fit.
  Such users will first wait to reach invariant [1]. */

  LOG_SYNC_POINT("log_wfs_after_reserving_before_buf_size_1");

  if (len > log.buf_size_sn.load()) {
    DBUG_EXECUTE_IF("ib_log_buffer_is_short_crash", DBUG_SUICIDE(););

    log_write_up_to(log, log_translate_sn_to_lsn(start_sn), false);

    /* Now the whole log has been written to disk up to start_sn,
    so there are no pending writes to log buffer for smaller sn. */
    LOG_SYNC_POINT("log_wfs_after_reserving_before_buf_size_2");

    /* Reservations for larger LSN could not increase size of log
    buffer as they could not have reached [1], because end_sn did
    not fit the log buffer (end_sn - start_sn > buf_size_sn), and
    next reservations would have their start_sn even greater. */
    ut_a(len > log.buf_size_sn.load());

    /* Note that the log.write_lsn could not be changed since it
    reached start_sn, until current thread continues and finishes
    writing its data to the log buffer.

    Note that any other thread will not attempt to write
    concurrently to the log buffer, because the log buffer
    represents range of sn:
            [start_sn, start_sn + log.buf_size_sn)
    and it holds:
            end_sn > start_sn + log_buf_size_sn.
    This will not change until we finished resizing log
    buffer and updated log.buf_size_sn, which therefore
    must happen at the very end of the resize procedure. */
    ut_a(log_translate_lsn_to_sn(log.write_lsn.load()) == start_sn);

    ib::info(ER_IB_MSG_1231)
        << "The transaction log size is too large"
        << " for srv_log_buffer_size (" << len << " > "
        << log.buf_size_sn.load() << "). Trying to extend it.";

    /* Resize without extra locking required.

    We cannot call log_buffer_resize() because it would try
    to acquire x-lock for the redo log and we keep s-lock.

    We already have ensured, that there are no possible
    concurrent writes to the log buffer. Note, we have also
    ensured that log writer finished writing up to start_sn.

    However, for extra safety, we prefer to acquire writer_mutex,
    and checkpointer_mutex. We consider this rare event. */

    log_checkpointer_mutex_enter(log);
    log_writer_mutex_enter(log);

    /* We multiply size at least by 1.382 to avoid case
    in which we keep resizing by few bytes only. */

    lsn_t new_lsn_size = log_translate_sn_to_lsn(
        static_cast<lsn_t>(1.382 * len + OS_FILE_LOG_BLOCK_SIZE));

    new_lsn_size = ut_uint64_align_up(new_lsn_size, OS_FILE_LOG_BLOCK_SIZE);

    log_buffer_resize_low(log, new_lsn_size, handle.start_lsn);

    log_writer_mutex_exit(log);
    log_checkpointer_mutex_exit(log);

  } else {
    /* Note that the size cannot get decreased.
    We are safe to continue. */
  }

  ut_a(len <= log.buf_size_sn.load());

  log_wait_for_space_in_log_buf(log, end_sn);
}

