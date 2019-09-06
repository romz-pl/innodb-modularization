#pragma once

#include <innodb/univ/univ.h>

struct trx_t;

enum trx_rseg_type_t {
  TRX_RSEG_TYPE_NONE = 0, /*!< void rollback segment type. */
  TRX_RSEG_TYPE_REDO,     /*!< redo rollback segment. */
  TRX_RSEG_TYPE_NOREDO    /*!< non-redo rollback segment. */
};
