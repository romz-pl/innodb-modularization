#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/srv_unix_file_flush_method.h>

#ifndef _WIN32

inline bool srv_is_direct_io() {
  return (srv_unix_file_flush_method == SRV_UNIX_O_DIRECT ||
          srv_unix_file_flush_method == SRV_UNIX_O_DIRECT_NO_FSYNC);
}

#else

inline bool srv_is_direct_io() {
  return (srv_win_file_flush_method == SRV_WIN_IO_UNBUFFERED);
}

#endif
