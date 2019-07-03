#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/os_file_t.h>

/** Common file descriptor for file IO instrumentation with PFS
on windows and other platforms */
struct pfs_os_file_t {
#ifdef UNIV_PFS_IO
  struct PSI_file *m_psi;
#else  /* UNIV_PFS_IO */
  pfs_os_file_t &operator=(os_file_t file) {
    m_file = file;
    return (*this);
  }
#endif /* UNIV_PFS_IO */

  os_file_t m_file;
};
