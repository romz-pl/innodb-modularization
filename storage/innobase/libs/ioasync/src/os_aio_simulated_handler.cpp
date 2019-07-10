#include <innodb/ioasync/os_aio_simulated_handler.h>

#include <innodb/ioasync/AIO.h>
#include <innodb/ioasync/SimulatedAIOHandler.h>
#include <innodb/ioasync/buf_page_cleaner_is_active.h>
#include <innodb/ioasync/os_aio_segment_wait_events.h>
#include <innodb/ioasync/srv_set_io_thread_op_info.h>
#include <innodb/ioasync/srv_shutdown_state.h>
#include <innodb/sync_event/os_event_reset.h>

/** Does simulated AIO. This function should be called by an i/o-handler
thread.

@param[in]	global_segment	The number of the segment in the aio arrays to
                                wait for; segment 0 is the ibuf i/o thread,
                                segment 1 the log i/o thread, then follow the
                                non-ibuf read threads, and as the last are the
                                non-ibuf write threads
@param[out]	m1		the messages passed with the AIO request; note
                                that also in the case where the AIO operation
                                failed, these output parameters are valid and
                                can be used to restart
                                the operation, for example
@param[out]	m2		Callback argument
@param[in]	type		IO context
@return DB_SUCCESS or error code */
dberr_t os_aio_simulated_handler(ulint global_segment, fil_node_t **m1,
                                        void **m2, IORequest *type) {
  Slot *slot;
  AIO *array;
  ulint segment;
  os_event_t event = os_aio_segment_wait_events[global_segment];

  segment = AIO::get_array_and_local_segment(&array, global_segment);

  SimulatedAIOHandler handler(array, segment);

  for (;;) {
    srv_set_io_thread_op_info(global_segment, "looking for i/o requests (a)");

    ulint n_slots = handler.check_pending(global_segment, event);

    if (n_slots == 0) {
      continue;
    }

    handler.init(n_slots);

    srv_set_io_thread_op_info(global_segment, "looking for i/o requests (b)");

    array->acquire();

    ulint n_reserved;

    slot = handler.check_completed(&n_reserved);

    if (slot != NULL) {
      break;

    } else if (n_reserved == 0
#ifndef UNIV_HOTBACKUP
               && !buf_page_cleaner_is_active &&
               srv_shutdown_state == SRV_SHUTDOWN_EXIT_THREADS
#endif /* !UNIV_HOTBACKUP */
    ) {

      /* There is no completed request. If there
      are no pending request at all, and the system
      is being shut down, exit. */

      array->release();

      *m1 = NULL;

      *m2 = NULL;

      return (DB_SUCCESS);

    } else if (handler.select()) {
      break;
    }

    /* No I/O requested at the moment */

    srv_set_io_thread_op_info(global_segment, "resetting wait event");

    /* We wait here until tbere are more IO requests
    for this segment. */

    os_event_reset(event);

    array->release();

    srv_set_io_thread_op_info(global_segment, "waiting for i/o request");

    os_event_wait(event);
  }

  /** Found a slot that has already completed its IO */

  if (slot == NULL) {
    /* Merge adjacent requests */
    handler.merge();

    /* Check if there are several consecutive blocks
    to read or write */

    srv_set_io_thread_op_info(global_segment, "consecutive i/o requests");

    // Note: We don't support write combining for simulated AIO.
    // ulint	total_len = handler.allocate_buffer();

    /* We release the array mutex for the time of the I/O: NOTE that
    this assumes that there is just one i/o-handler thread serving
    a single segment of slots! */

    array->release();

    // Note: We don't support write combining for simulated AIO.
    // handler.copy_to_buffer(total_len);

    srv_set_io_thread_op_info(global_segment, "doing file i/o");

    handler.io();

    srv_set_io_thread_op_info(global_segment, "file i/o done");

    handler.io_complete();

    array->acquire();

    handler.done();

    /* We return the messages for the first slot now, and if there
    were several slots, the messages will be returned with
    subsequent calls of this function */

    slot = handler.first_slot();
  }

  ut_ad(slot->is_reserved);

  *m1 = slot->m1;
  *m2 = slot->m2;

  *type = slot->type;

  array->release(slot);

  array->release();

  return (DB_SUCCESS);
}



