#include <innodb/io/pfs_os_file_write_int_fd_func.h>

#ifdef UNIV_PFS_IO

#include <innodb/io/os_file_write_func.h>
#include <innodb/io/IORequest.h>
#include <innodb/io/OS_FILE_FROM_FD.h>

/** NOTE! Please use the corresponding macro os_file_write(), not
directly this function!
This is the performance schema instrumented wrapper function for
os_file_write() which requests a synchronous write operation.
@param[in, out] type            IO request context
@param[in]      name            Name of the file or path as NUL terminated
                                string
@param[in]      file            Open file handle
@param[out]     buf             buffer where to read
@param[in]      offset          file offset where to read
@param[in]      n               number of bytes to read
@param[in]      src_file        file name where func invoked
@param[in]      src_line        line where the func invoked
@return DB_SUCCESS if request was successful */
dberr_t pfs_os_file_write_int_fd_func(IORequest &type, const char *name,
                                      int file, const void *buf,
                                      os_offset_t offset, ulint n,
                                      const char *src_file, ulint src_line) {
  PSI_file_locker_state state;
  struct PSI_file_locker *locker = NULL;

  locker = PSI_FILE_CALL(get_thread_file_descriptor_locker)(&state, file,
                                                            PSI_FILE_WRITE);
  if (locker != NULL) {
    PSI_FILE_CALL(start_file_wait)(locker, n, __FILE__, __LINE__);
  }
  dberr_t result =
      os_file_write_func(type, name, OS_FILE_FROM_FD(file), buf, offset, n);

  if (locker != NULL) {
    PSI_FILE_CALL(end_file_wait)(locker, n);
  }

  return (result);
}


#endif
