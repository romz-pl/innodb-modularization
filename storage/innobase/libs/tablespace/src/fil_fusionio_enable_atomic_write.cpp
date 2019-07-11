#include <innodb/tablespace/fil_fusionio_enable_atomic_write.h>

#if !defined(NO_FALLOCATE) && defined(UNIV_LINUX)

#include <innodb/assert/assert.h>
#include <innodb/io/pfs_os_file_t.h>
#include <innodb/io/srv_unix_file_flush_method.h>

#include <sys/ioctl.h>

/** FusionIO atomic write control info */
#define DFS_IOCTL_ATOMIC_WRITE_SET _IOW(0x95, 2, uint)

/** Try and enable FusionIO atomic writes.
@param[in] file		OS file handle
@return true if successful */
bool fil_fusionio_enable_atomic_write(pfs_os_file_t file) {
  if (srv_unix_file_flush_method == SRV_UNIX_O_DIRECT) {
    uint atomic = 1;

    ut_a(file.m_file != -1);

    if (ioctl(file.m_file, DFS_IOCTL_ATOMIC_WRITE_SET, &atomic) != -1) {
      return (true);
    }
  }

  return (false);
}
#endif /* !NO_FALLOCATE && UNIV_LINUX */
