#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/pfs_os_file_t.h>

/** Types for file create @{ */
const ulint OS_DATA_FILE = 100;
const ulint OS_LOG_FILE = 101;
/* Don't use this for Data files, Log files. Use it for smaller files
or if number of bytes to write are not multiple of sector size.
With this flag, writes to file will be always buffered and ignores the value
of innodb_flush_method. */
const ulint OS_BUFFERED_FILE = 102;

const ulint OS_CLONE_DATA_FILE = 103;
const ulint OS_CLONE_LOG_FILE = 104;
/* @} */


/* Options for file_create */
const ulint OS_FILE_AIO = 61;
const ulint OS_FILE_NORMAL = 62;
/* @} */


pfs_os_file_t os_file_create_func(const char *name, ulint create_mode,
                                  ulint purpose, ulint type, bool read_only,
                                  bool *success) MY_ATTRIBUTE((warn_unused_result));
