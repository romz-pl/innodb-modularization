#include <innodb/ioasync/os_aio_linux_handler.h>

#if defined(LINUX_NATIVE_AIO)

#include <innodb/ioasync/LinuxAIOHandler.h>
#include <innodb/tablespace/fil_no_punch_hole.h>

/** This function is only used in Linux native asynchronous i/o.
Waits for an aio operation to complete. This function is used to wait for
the completed requests. The aio array of pending requests is divided
into segments. The thread specifies which segment or slot it wants to wait
for. NOTE: this function will also take care of freeing the aio slot,
therefore no other thread is allowed to do the freeing!

@param[in]	global_segment	segment number in the aio array
                                to wait for; segment 0 is the ibuf
                                i/o thread, segment 1 is log i/o thread,
                                then follow the non-ibuf read threads,
                                and the last are the non-ibuf write
                                threads.
@param[out]	m1		the messages passed with the
@param[out]	m2			AIO request; note that in case the
                                AIO operation failed, these output
                                parameters are valid and can be used to
                                restart the operation.
@param[out]	request		IO context
@return DB_SUCCESS if the IO was successful */
dberr_t os_aio_linux_handler(ulint global_segment, fil_node_t **m1,
                                    void **m2, IORequest *request) {
  LinuxAIOHandler handler(global_segment);

  dberr_t err = handler.poll(m1, m2, request);

  if (err == DB_IO_NO_PUNCH_HOLE) {
    fil_no_punch_hole(*m1);
    err = DB_SUCCESS;
  }

  return (err);
}

#endif /* LINUX_NATIVE_AIO */

