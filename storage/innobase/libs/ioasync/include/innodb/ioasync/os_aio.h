#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_PFS_IO

#include <innodb/ioasync/pfs_os_aio_func.h>

#define os_aio(type, mode, name, file, buf, offset, n, read_only, message1,    \
               message2)                                                       \
  pfs_os_aio_func(type, mode, name, file, buf, offset, n, read_only, message1, \
                  message2, __FILE__, __LINE__)

#else

#include <innodb/ioasync/os_aio_func.h>

#define os_aio(type, mode, name, file, buf, offset, n, read_only, message1, \
               message2)                                                    \
  os_aio_func(type, mode, name, file, buf, offset, n, read_only, message1,  \
              message2)


#endif /* UNIV_PFS_IO */
