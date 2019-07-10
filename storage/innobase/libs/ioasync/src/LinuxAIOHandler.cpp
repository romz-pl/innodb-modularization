#include <innodb/ioasync/LinuxAIOHandler.h>

#if defined(LINUX_NATIVE_AIO)

#include <innodb/ioasync/srv_set_io_thread_op_info.h>
#include <innodb/io/os_file_handle_error.h>
#include <innodb/ioasync/AIOHandler.h>
#include <innodb/logger/fatal.h>

/** Resubmit an IO request that was only partially successful
@param[in,out]	slot		Request to resubmit
@return DB_SUCCESS or DB_FAIL if the IO resubmit request failed */
dberr_t LinuxAIOHandler::resubmit(Slot *slot) {
#ifdef UNIV_DEBUG
  /* Bytes already read/written out */
  ulint n_bytes = slot->ptr - slot->buf;

  ut_ad(m_array->is_mutex_owned());

  ut_ad(n_bytes < slot->original_len);
  ut_ad(static_cast<ulint>(slot->n_bytes) < slot->original_len - n_bytes);
  /* Partial read or write scenario */
  ut_ad(slot->len >= static_cast<ulint>(slot->n_bytes));
#endif /* UNIV_DEBUG */

  slot->len -= slot->n_bytes;
  slot->ptr += slot->n_bytes;
  slot->offset += slot->n_bytes;

  /* Resetting the bytes read/written */
  slot->n_bytes = 0;
  slot->io_already_done = false;

  /* make sure that slot->offset fits in off_t */
  ut_ad(sizeof(off_t) >= sizeof(os_offset_t));
  struct iocb *iocb = &slot->control;
  if (slot->type.is_read()) {
    io_prep_pread(iocb, slot->file.m_file, slot->ptr, slot->len, slot->offset);

  } else {
    ut_a(slot->type.is_write());

    io_prep_pwrite(iocb, slot->file.m_file, slot->ptr, slot->len, slot->offset);
  }
  iocb->data = slot;

  /* Resubmit an I/O request */
  int ret = io_submit(m_array->io_ctx(m_segment), 1, &iocb);

  if (ret < -1) {
    errno = -ret;
  }

  return (ret < 0 ? DB_IO_PARTIAL_FAILED : DB_SUCCESS);
}

/** Check if the AIO succeeded
@param[in,out]	slot		The slot to check
@return DB_SUCCESS, DB_FAIL if the operation should be retried or
        DB_IO_ERROR on all other errors */
dberr_t LinuxAIOHandler::check_state(Slot *slot) {
  ut_ad(m_array->is_mutex_owned());

  /* Note that it may be that there is more then one completed
  IO requests. We process them one at a time. We may have a case
  here to improve the performance slightly by dealing with all
  requests in one sweep. */

  srv_set_io_thread_op_info(m_global_segment,
                            "processing completed aio requests");

  ut_ad(slot->io_already_done);

  dberr_t err;

  if (slot->ret == 0) {
    err = AIOHandler::post_io_processing(slot);

  } else {
    errno = -slot->ret;

    /* os_file_handle_error does tell us if we should retry
    this IO. As it stands now, we don't do this retry when
    reaping requests from a different context than
    the dispatcher. This non-retry logic is the same for
    Windows and Linux native AIO.
    We should probably look into this to transparently
    re-submit the IO. */
    os_file_handle_error(slot->name, "Linux aio");

    err = DB_IO_ERROR;
  }

  return (err);
}

/** If no slot was found then the m_array->m_mutex will be released.
@param[out]	n_pending		The number of pending IOs
@return NULL or a slot that has completed IO */
Slot *LinuxAIOHandler::find_completed_slot(ulint *n_pending) {
  ulint offset = m_n_slots * m_segment;

  *n_pending = 0;

  m_array->acquire();

  Slot *slot = m_array->at(offset);

  for (ulint i = 0; i < m_n_slots; ++i, ++slot) {
    if (slot->is_reserved) {
      ++*n_pending;

      if (slot->io_already_done) {
        /* Something for us to work on.
        Note: We don't release the mutex. */
        return (slot);
      }
    }
  }

  m_array->release();

  return (NULL);
}

/** This function is only used in Linux native asynchronous i/o. This is
called from within the io-thread. If there are no completed IO requests
in the slot array, the thread calls this function to collect more
requests from the kernel.
The io-thread waits on io_getevents(), which is a blocking call, with
a timeout value. Unless the system is very heavy loaded, keeping the
io-thread very busy, the io-thread will spend most of its time waiting
in this function.
The io-thread also exits in this function. It checks server status at
each wakeup and that is why we use timed wait in io_getevents(). */
void LinuxAIOHandler::collect() {

  /** timeout for each io_getevents() call = 500ms. */
  static const ulint OS_AIO_REAP_TIMEOUT = 500000000UL;

  ut_ad(m_n_slots > 0);
  ut_ad(m_array != NULL);
  ut_ad(m_segment < m_array->get_n_segments());

  /* Which io_context we are going to use. */
  io_context *io_ctx = m_array->io_ctx(m_segment);

  /* Starting point of the m_segment we will be working on. */
  ulint start_pos = m_segment * m_n_slots;

  /* End point. */
  ulint end_pos = start_pos + m_n_slots;

  for (;;) {
    struct io_event *events;

    /* Which part of event array we are going to work on. */
    events = m_array->io_events(m_segment * m_n_slots);

    /* Initialize the events. */
    memset(events, 0, sizeof(*events) * m_n_slots);

    /* The timeout value is arbitrary. We probably need
    to experiment with it a little. */
    struct timespec timeout;

    timeout.tv_sec = 0;
    timeout.tv_nsec = OS_AIO_REAP_TIMEOUT;

    int ret;

    ret = io_getevents(io_ctx, 1, m_n_slots, events, &timeout);

    for (int i = 0; i < ret; ++i) {
      struct iocb *iocb;

      iocb = reinterpret_cast<struct iocb *>(events[i].obj);
      ut_a(iocb != NULL);

      Slot *slot = reinterpret_cast<Slot *>(iocb->data);

      /* Some sanity checks. */
      ut_a(slot != NULL);
      ut_a(slot->is_reserved);

      /* We are not scribbling previous segment. */
      ut_a(slot->pos >= start_pos);

      /* We have not overstepped to next segment. */
      ut_a(slot->pos < end_pos);

      /* We never compress/decompress the first page */

      if (slot->offset > 0 && !slot->skip_punch_hole &&
          slot->type.is_compression_enabled() && !slot->type.is_log() &&
          slot->type.is_write() && slot->type.is_compressed() &&
          slot->type.punch_hole()) {
        slot->err = AIOHandler::io_complete(slot);
      } else {
        slot->err = DB_SUCCESS;
      }

      /* Mark this request as completed. The error handling
      will be done in the calling function. */
      m_array->acquire();

      /* events[i].res2 should always be ZERO */
      ut_ad(events[i].res2 == 0);
      slot->io_already_done = true;

      /*Even though events[i].res is an unsigned number in libaio, it is
      used to return a negative value (negated errno value) to indicate
      error and a positive value to indicate number of bytes read or
      written. */

      if (events[i].res > slot->len) {
        /* failure */
        slot->n_bytes = 0;
        slot->ret = events[i].res;
      } else {
        /* success */
        slot->n_bytes = events[i].res;
        slot->ret = 0;
      }
      m_array->release();
    }

    if (srv_shutdown_state == SRV_SHUTDOWN_EXIT_THREADS ||
        !buf_page_cleaner_is_active || ret > 0) {
      break;
    }

    /* This error handling is for any error in collecting the
    IO requests. The errors, if any, for any particular IO
    request are simply passed on to the calling routine. */

    switch (ret) {
      case -EAGAIN:
        /* Not enough resources! Try again. */

      case -EINTR:
        /* Interrupted! The behaviour in case of an interrupt.
        If we have some completed IOs available then the
        return code will be the number of IOs. We get EINTR
        only if there are no completed IOs and we have been
        interrupted. */

      case 0:
        /* No pending request! Go back and check again. */

        continue;
    }

    /* All other errors should cause a trap for now. */
    ib::fatal(ER_IB_MSG_755)
        << "Unexpected ret_code[" << ret << "] from io_getevents()!";

    break;
  }
}

/** Process a Linux AIO request
@param[out]	m1		the messages passed with the
@param[out]	m2		AIO request; note that in case the
                                AIO operation failed, these output
                                parameters are valid and can be used to
                                restart the operation.
@param[out]	request		IO context
@return DB_SUCCESS or error code */
dberr_t LinuxAIOHandler::poll(fil_node_t **m1, void **m2, IORequest *request) {
  dberr_t err;
  Slot *slot;

  /* Loop until we have found a completed request. */
  for (;;) {
    ulint n_pending;

    slot = find_completed_slot(&n_pending);

    if (slot != NULL) {
      ut_ad(m_array->is_mutex_owned());

      err = check_state(slot);

      /* DB_FAIL is not a hard error, we should retry */
      if (err != DB_FAIL) {
        break;
      }

      /* Partial IO, resubmit request for
      remaining bytes to read/write */
      err = resubmit(slot);

      if (err != DB_SUCCESS) {
        break;
      }

      m_array->release();

    } else if (is_shutdown() && n_pending == 0) {
      /* There is no completed request. If there is
      no pending request at all, and the system is
      being shut down, exit. */

      *m1 = NULL;
      *m2 = NULL;

      return (DB_SUCCESS);

    } else {
      /* Wait for some request. Note that we return
      from wait if we have found a request. */

      srv_set_io_thread_op_info(m_global_segment,
                                "waiting for completed aio requests");

      collect();
    }
  }

  if (err == DB_IO_PARTIAL_FAILED) {
    /* Aborting in case of submit failure */
    ib::fatal(ER_IB_MSG_756) << "Native Linux AIO interface. "
                                "io_submit() call failed when "
                                "resubmitting a partial I/O "
                                "request on the file "
                             << slot->name << ".";
  }

  *m1 = slot->m1;
  *m2 = slot->m2;

  *request = slot->type;

  m_array->release(slot);

  m_array->release();

  return (err);
}


#endif
