#pragma once

#include <innodb/univ/univ.h>

/** Transaction execution states when trx->state == TRX_STATE_ACTIVE */
enum trx_que_t {
  TRX_QUE_RUNNING,      /*!< transaction is running */
  TRX_QUE_LOCK_WAIT,    /*!< transaction is waiting for
                        a lock */
  TRX_QUE_ROLLING_BACK, /*!< transaction is rolling back */
  TRX_QUE_COMMITTING    /*!< transaction is committing */
};
