#include <innodb/ioasync/os_aio_init.h>

#include <innodb/ioasync/AIO.h>
#include <innodb/io/os_fusionio_get_sector_size.h>
#include <innodb/io/macros.h>

/** Initializes the asynchronous io system. Creates one array each for ibuf
and log i/o. Also creates one array each for read and write where each
array is divided logically into n_readers and n_writers
respectively. The caller must create an i/o handler thread for each
segment in these arrays. This function also creates the sync array.
No i/o handler thread needs to be created for that
@param[in]	n_readers	number of reader threads
@param[in]	n_writers	number of writer threads
@param[in]	n_slots_sync	number of slots in the sync aio array */
bool os_aio_init(ulint n_readers, ulint n_writers, ulint n_slots_sync) {
  /* Maximum number of pending aio operations allowed per segment */
  ulint limit = 8 * OS_AIO_N_PENDING_IOS_PER_THREAD;

#ifdef _WIN32
  if (srv_use_native_aio) {
    limit = SRV_N_PENDING_IOS_PER_THREAD;
  }
#endif /* _WIN32 */

    /* Get sector size for DIRECT_IO. In this case, we need to
    know the sector size for aligning the write buffer. */
#if !defined(NO_FALLOCATE) && defined(UNIV_LINUX)
  os_fusionio_get_sector_size();
#endif /* !NO_FALLOCATE && UNIV_LINUX */

  return (AIO::start(limit, n_readers, n_writers, n_slots_sync));
}
