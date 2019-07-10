#include <innodb/ioasync/pfs_os_aio_func.h>

#ifdef UNIV_PFS_IO
#ifndef UNIV_HOTBACKUP


#include <innodb/assert/assert.h>
#include <innodb/io/register_pfs_file_io_begin.h>
#include <innodb/io/register_pfs_file_io_end.h>
#include <innodb/ioasync/os_aio_func.h>
#include <innodb/io/IORequest.h>

#include "mysql/components/services/psi_file_bits.h"


/** NOTE! Please use the corresponding macro os_aio(), not directly this
function!
Performance schema wrapper function of os_aio() which requests
an asynchronous i/o operation.
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
                                if mode is OS_AIO_SYNC
@param[in,out]	m2		message for the AIO handler (can be used to
                                identify a completed AIO operation); ignored
                                if mode is OS_AIO_SYNC
@param[in]	src_file	file name where func invoked
@param[in]	src_line	line where the func invoked
@return DB_SUCCESS if request was queued successfully, false if fail */
dberr_t pfs_os_aio_func(IORequest &type, AIO_mode aio_mode, const char *name,
                        pfs_os_file_t file, void *buf, os_offset_t offset,
                        ulint n, bool read_only, fil_node_t *m1, void *m2,
                        const char *src_file, uint src_line) {
  PSI_file_locker_state state;
  struct PSI_file_locker *locker = NULL;

  ut_ad(type.validate());

  /* Register the read or write I/O depending on "type" */
  register_pfs_file_io_begin(&state, locker, file, n,
                             type.is_write() ? PSI_FILE_WRITE : PSI_FILE_READ,
                             src_file, src_line);

  dberr_t result = os_aio_func(type, aio_mode, name, file, buf, offset, n,
                               read_only, m1, m2);

  register_pfs_file_io_end(locker, n);

  return (result);
}
#endif /* UNIV_HOTBACKUP */
#endif /* UNIV_PFS_IO */
