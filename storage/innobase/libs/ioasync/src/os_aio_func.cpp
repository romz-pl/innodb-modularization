#include <innodb/ioasync/os_aio_func.h>

#include <innodb/ioasync/AIO.h>
#include <innodb/io/srv_use_native_aio.h>
#include <innodb/io/os_file_read_func.h>
#include <innodb/io/os_file_write_func.h>
#include <innodb/io/os_n_file_reads.h>
#include <innodb/io/os_n_file_writes.h>
#include <innodb/io/os_bytes_read_since_printout.h>
#include <innodb/io/os_file_handle_error.h>

/**
NOTE! Use the corresponding macro os_aio(), not directly this function!
Requests an asynchronous i/o operation.
@param[in]	type		IO request context
@param[in]	aio_mode	IO mode
@param[in]	name		Name of the file or path as NUL terminated
                                string
@param[in]	file		Open file handle
@param[out]	buf		buffer where to read
@param[in]	offset		file offset where to read
@param[in]	n		number of bytes to read
@param[in]	read_only	if true read only mode checks are enforced
@param[in,out]	m1		Message for the AIO handler, (can be used to
                                identify a completed AIO operation); ignored
                                if mode is AIO_mode::SYNC
@param[in,out]	m2		message for the AIO handler (can be used to
                                identify a completed AIO operation); ignored
                                if mode is AIO_mode::SYNC
@return DB_SUCCESS or error code */
dberr_t os_aio_func(IORequest &type, AIO_mode aio_mode, const char *name,
                    pfs_os_file_t file, void *buf, os_offset_t offset, ulint n,
                    bool read_only, fil_node_t *m1, void *m2) {
#ifdef WIN_ASYNC_IO
  BOOL ret = TRUE;
#endif /* WIN_ASYNC_IO */

  ut_ad(n > 0);
  ut_ad((n % OS_FILE_LOG_BLOCK_SIZE) == 0);
  ut_ad((offset % OS_FILE_LOG_BLOCK_SIZE) == 0);
#ifndef UNIV_HOTBACKUP
  ut_ad(os_aio_validate_skip());
#endif /* !UNIV_HOTBACKUP */

#ifdef WIN_ASYNC_IO
  ut_ad((n & 0xFFFFFFFFUL) == n);
#endif /* WIN_ASYNC_IO */

  if (aio_mode == AIO_mode::SYNC
#ifdef WIN_ASYNC_IO
      && !srv_use_native_aio
#endif /* WIN_ASYNC_IO */
  ) {
    /* This is actually an ordinary synchronous read or write:
    no need to use an i/o-handler thread. NOTE that if we use
    Windows async i/o, Windows does not allow us to use
    ordinary synchronous os_file_read etc. on the same file,
    therefore we have built a special mechanism for synchronous
    wait in the Windows case.
    Also note that the Performance Schema instrumentation has
    been performed by current os_aio_func()'s wrapper function
    pfs_os_aio_func(). So we would no longer need to call
    Performance Schema instrumented os_file_read() and
    os_file_write(). Instead, we should use os_file_read_func()
    and os_file_write_func() */

    if (type.is_read()) {
      return (os_file_read_func(type, file.m_file, buf, offset, n));
    }

    ut_ad(type.is_write());
    return (os_file_write_func(type, name, file.m_file, buf, offset, n));
  }

try_again:

  AIO *array;

  array = AIO::select_slot_array(type, read_only, aio_mode);

  Slot *slot;

  slot = array->reserve_slot(type, m1, m2, file, name, buf, offset, n);

  if (type.is_read()) {
    if (srv_use_native_aio) {
      ++os_n_file_reads;

      os_bytes_read_since_printout += n;
#ifdef WIN_ASYNC_IO
      ret = ReadFile(file.m_file, slot->ptr, slot->len, &slot->n_bytes,
                     &slot->control);
#elif defined(LINUX_NATIVE_AIO)
      if (!array->linux_dispatch(slot)) {
        goto err_exit;
      }
#endif /* WIN_ASYNC_IO */
    } else if (type.is_wake()) {
      AIO::wake_simulated_handler_thread(
          AIO::get_segment_no_from_slot(array, slot));
    }
  } else if (type.is_write()) {
    if (srv_use_native_aio) {
      ++os_n_file_writes;

#ifdef WIN_ASYNC_IO
      ret = WriteFile(file.m_file, slot->ptr, slot->len, &slot->n_bytes,
                      &slot->control);
#elif defined(LINUX_NATIVE_AIO)
      if (!array->linux_dispatch(slot)) {
        goto err_exit;
      }
#endif /* WIN_ASYNC_IO */

    } else if (type.is_wake()) {
      AIO::wake_simulated_handler_thread(
          AIO::get_segment_no_from_slot(array, slot));
    }
  } else {
    ut_error;
  }

#ifdef WIN_ASYNC_IO
  if (srv_use_native_aio) {
    if ((ret && slot->len == slot->n_bytes) ||
        (!ret && GetLastError() == ERROR_IO_PENDING)) {
      /* AIO was queued successfully! */

      if (aio_mode == AIO_mode::SYNC) {
        IORequest dummy_type;
        void *dummy_mess2;
        struct fil_node_t *dummy_mess1;

        /* We want a synchronous i/o operation on a
        file where we also use async i/o: in Windows
        we must use the same wait mechanism as for
        async i/o */

        return (os_aio_windows_handler(ULINT_UNDEFINED, slot->pos, &dummy_mess1,
                                       &dummy_mess2, &dummy_type));
      }

      return (DB_SUCCESS);
    }

    goto err_exit;
  }
#endif /* WIN_ASYNC_IO */

  /* AIO request was queued successfully! */
  return (DB_SUCCESS);

#if defined LINUX_NATIVE_AIO || defined WIN_ASYNC_IO
err_exit:
#endif /* LINUX_NATIVE_AIO || WIN_ASYNC_IO */

  array->release_with_mutex(slot);

  if (os_file_handle_error(name, type.is_read() ? "aio read" : "aio write")) {
    goto try_again;
  }

  return (DB_IO_ERROR);
}
