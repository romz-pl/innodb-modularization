#pragma once

#include <innodb/univ/univ.h>

/** Supported redo log formats. Stored in LOG_HEADER_FORMAT. */
enum log_header_format_t {
  /** The MySQL 5.7.9 redo log format identifier. We can support recovery
  from this format if the redo log is clean (logically empty). */
  LOG_HEADER_FORMAT_5_7_9 = 1,

  /** Remove MLOG_FILE_NAME and MLOG_CHECKPOINT, introduce MLOG_FILE_OPEN
  redo log record. */
  LOG_HEADER_FORMAT_8_0_1 = 2,

  /** Allow checkpoint_lsn to point any data byte within redo log (before
  it had to point the beginning of a group of log records). */
  LOG_HEADER_FORMAT_8_0_3 = 3,

  /** The redo log format identifier
  corresponding to the current format version. */
  LOG_HEADER_FORMAT_CURRENT = LOG_HEADER_FORMAT_8_0_3
};
