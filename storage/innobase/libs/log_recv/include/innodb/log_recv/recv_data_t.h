#pragma once

#include <innodb/univ/univ.h>

/** Block of log record data */
struct recv_data_t {
  /** pointer to the next block or NULL.  The log record data
  is stored physically immediately after this struct, max amount
  RECV_DATA_BLOCK_SIZE bytes of it */

  recv_data_t *next;
};
