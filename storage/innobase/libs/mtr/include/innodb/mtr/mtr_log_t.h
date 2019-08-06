#pragma once

#include <innodb/univ/univ.h>

/** Logging modes for a mini-transaction */
enum mtr_log_t {
  /** Default mode: log all operations modifying disk-based data */
  MTR_LOG_ALL = 21,

  /** Log no operations and dirty pages are not added to the flush list */
  MTR_LOG_NONE = 22,

  /** Don't generate REDO log but add dirty pages to flush list */
  MTR_LOG_NO_REDO = 23,

  /** Inserts are logged in a shorter form */
  MTR_LOG_SHORT_INSERTS = 24
};
