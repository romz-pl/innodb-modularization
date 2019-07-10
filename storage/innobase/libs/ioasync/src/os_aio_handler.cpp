#include <innodb/ioasync/os_aio_handler.h>

#include <innodb/ioasync/srv_set_io_thread_op_info.h>
#include <innodb/ioasync/os_aio_linux_handler.h>
#include <innodb/ioasync/srv_set_io_thread_op_info.h>
#include <innodb/ioasync/os_aio_simulated_handler.h>
#include <innodb/io/srv_use_native_aio.h>

/** Waits for an AIO operation to complete. This function is used to wait
for completed requests. The aio array of pending requests is divided
into segments. The thread specifies which segment or slot it wants to wait
for. NOTE: this function will also take care of freeing the aio slot,
therefore no other thread is allowed to do the freeing!
@param[in]	segment		The number of the segment in the aio arrays to
                                wait for; segment 0 is the ibuf I/O thread,
                                segment 1 the log I/O thread, then follow the
                                non-ibuf read threads, and as the last are the
                                non-ibuf write threads; if this is
                                ULINT_UNDEFINED, then it means that sync AIO
                                is used, and this parameter is ignored
@param[out]	m1		the messages passed with the AIO request; note
                                that also in the case where the AIO operation
                                failed, these output parameters are valid and
                                can be used to restart the operation,
                                for example
@param[out]	m2		callback message
@param[out]	request		OS_FILE_WRITE or ..._READ
@return DB_SUCCESS or error code */
dberr_t os_aio_handler(ulint segment, fil_node_t **m1, void **m2,
                       IORequest *request) {
  dberr_t err;

  if (srv_use_native_aio) {
    srv_set_io_thread_op_info(segment, "native aio handle");

#ifdef WIN_ASYNC_IO

    err = os_aio_windows_handler(segment, 0, m1, m2, request);

#elif defined(LINUX_NATIVE_AIO)

    err = os_aio_linux_handler(segment, m1, m2, request);

#else
    ut_error;

    err = DB_ERROR; /* Eliminate compiler warning */

#endif /* WIN_ASYNC_IO */

  } else {
    srv_set_io_thread_op_info(segment, "simulated aio handle");

    err = os_aio_simulated_handler(segment, m1, m2, request);
  }

  return (err);
}


