#pragma once

#include <innodb/univ/univ.h>

/** Alternatives for the file flush option in Unix.
@see innodb_flush_method_names */
enum srv_unix_flush_t {
  SRV_UNIX_FSYNC = 0,  /*!< fsync, the default */
  SRV_UNIX_O_DSYNC,    /*!< open log files in O_SYNC mode */
  SRV_UNIX_LITTLESYNC, /*!< do not call os_file_flush()
                       when writing data files, but do flush
                       after writing to log files */
  SRV_UNIX_NOSYNC,     /*!< do not flush after writing */
  SRV_UNIX_O_DIRECT,   /*!< invoke os_file_set_nocache() on
                       data files. This implies using
                       non-buffered IO but still using fsync,
                       the reason for which is that some FS
                       do not flush meta-data when
                       unbuffered IO happens */
  SRV_UNIX_O_DIRECT_NO_FSYNC
  /*!< do not use fsync() when using
  direct IO i.e.: it can be set to avoid
  the fsync() call that we make when
  using SRV_UNIX_O_DIRECT. However, in
  this case user/DBA should be sure about
  the integrity of the meta-data */
};
