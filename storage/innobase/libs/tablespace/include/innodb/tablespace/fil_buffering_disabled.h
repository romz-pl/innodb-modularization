#pragma once

#include <innodb/univ/univ.h>

#include <innodb/tablespace/fil_type_t.h>
#include <innodb/io/srv_unix_file_flush_method.h>
#include <innodb/io/srv_unix_flush_t.h>

/** Determine if user has explicitly disabled fsync(). */
#ifndef _WIN32
#define fil_buffering_disabled(s)         \
  ((s)->purpose == FIL_TYPE_TABLESPACE && \
   srv_unix_file_flush_method == SRV_UNIX_O_DIRECT_NO_FSYNC)
#else /* _WIN32 */
#define fil_buffering_disabled(s) (0)
#endif /* _WIN32 */

