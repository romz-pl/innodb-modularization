#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_trx/trx_t.h>
#include <innodb/trx_trx/trx_is_autocommit_non_locking.h>
#include <innodb/error/ut_error.h>

/**
Check transaction state */
#define check_trx_state(t)                      \
  do {                                          \
    ut_ad(!trx_is_autocommit_non_locking((t))); \
    switch ((t)->state) {                       \
      case TRX_STATE_PREPARED:                  \
        /* fall through */                      \
      case TRX_STATE_ACTIVE:                    \
      case TRX_STATE_COMMITTED_IN_MEMORY:       \
        continue;                               \
      case TRX_STATE_NOT_STARTED:               \
      case TRX_STATE_FORCED_ROLLBACK:           \
        break;                                  \
    }                                           \
    ut_error;                                   \
  } while (0)
