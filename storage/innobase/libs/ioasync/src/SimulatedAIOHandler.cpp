#include <innodb/ioasync/SimulatedAIOHandler.h>

#include <innodb/ioasync/srv_set_io_thread_op_info.h>
#include <innodb/ioasync/os_aio_recommend_sleep_for_read_threads.h>

/** Wait for I/O requests
@return the number of slots */
ulint SimulatedAIOHandler::check_pending(ulint global_segment,
                                         os_event_t event) {
/* NOTE! We only access constant fields in os_aio_array.
Therefore we do not have to acquire the protecting mutex yet */

#ifndef UNIV_HOTBACKUP
  ut_ad(os_aio_validate_skip());
#endif /* !UNIV_HOTBACKUP */

  ut_ad(m_segment < m_array->get_n_segments());

  /* Look through n slots after the segment * n'th slot */

  if (AIO::is_read(m_array) && os_aio_recommend_sleep_for_read_threads) {
    /* Give other threads chance to add several
    I/Os to the array at once. */

    srv_set_io_thread_op_info(global_segment, "waiting for i/o request");

    os_event_wait(event);

    return (0);
  }

  return (m_array->slots_per_segment());
}

