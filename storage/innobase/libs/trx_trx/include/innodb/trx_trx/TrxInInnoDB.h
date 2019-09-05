#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_trx/trx_t.h>
#include <innodb/trx_types/flags.h>
#include <innodb/thread/os_thread_get_curr_id.h>
#include <innodb/io/srv_read_only_mode.h>
#include <innodb/trx_trx/trx_mutex_own.h>
#include <innodb/trx_trx/trx_mutex_enter.h>
#include <innodb/trx_trx/trx_mutex_exit.h>
#include <innodb/trx_trx/trx_is_autocommit_non_locking.h>
#include <innodb/trx_trx/trx_is_started.h>


/** Track if a transaction is executing inside InnoDB code. It acts
like a gate between the Server and InnoDB.  */
class TrxInInnoDB {
 public:
  /**
  @param[in,out] trx	Transaction entering InnoDB via the handler
  @param[in] disable	true if called from COMMIT/ROLLBACK method */
  TrxInInnoDB(trx_t *trx, bool disable = false) : m_trx(trx) {
    enter(trx, disable);
  }

  /**
  Destructor */
  ~TrxInInnoDB() { exit(m_trx); }

  /**
  @return true if the transaction has been marked for asynchronous
          rollback */
  bool is_aborted() const { return (is_aborted(m_trx)); }

  /**
  @return true if the transaction can't be rolled back asynchronously */
  bool is_rollback_disabled() const {
    return ((m_trx->in_innodb & TRX_FORCE_ROLLBACK_DISABLE) > 0);
  }

  /**
  @return true if the transaction has been marked for asynchronous
          rollback */
  static bool is_aborted(const trx_t *trx) {
    if (trx->state == TRX_STATE_NOT_STARTED) {
      return (false);
    }

    ut_ad(srv_read_only_mode || trx->in_depth > 0);
    ut_ad(srv_read_only_mode || trx->in_innodb > 0);

    return (trx->abort || trx->state == TRX_STATE_FORCED_ROLLBACK);
  }

  /**
  Start statement requested for transaction.
  @param[in, out] trx	Transaction at the start of a SQL statement */
  static void begin_stmt(trx_t *trx) { enter(trx, false); }

  /**
  Note an end statement for transaction
  @param[in, out] trx	Transaction at end of a SQL statement */
  static void end_stmt(trx_t *trx) { exit(trx); }

  /**
  @return true if the rollback is being initiated by the thread that
          marked the transaction for asynchronous rollback */
  static bool is_async_rollback(const trx_t *trx) {
    return (trx->killed_by == os_thread_get_curr_id());
  }

 private:
  /** Note that we have crossed into InnoDB code.
  @param[in]	trx	transaction
  @param[in]	disable	true if called from COMMIT/ROLLBACK method */
  static void enter(trx_t *trx, bool disable) {
    if (srv_read_only_mode) {
      return;
    }

    ut_ad(!is_async_rollback(trx));

    /* If it hasn't already been marked for async rollback.
    and it will be committed/rolled back. */
    if (disable) {
      trx_mutex_enter(trx);
      if (!is_forced_rollback(trx) && is_started(trx) &&
          !trx_is_autocommit_non_locking(trx)) {
        ut_ad(trx->killed_by == 0);

        /* This transaction has crossed the point of
        no return and cannot be rolled back
        asynchronously now. It must commit or rollback
        synhronously. */

        trx->in_innodb |= TRX_FORCE_ROLLBACK_DISABLE;
      }
      trx_mutex_exit(trx);
    }

    /* Avoid excessive mutex acquire/release */
    ++trx->in_depth;

    /* If trx->in_depth is greater than 1 then
    transaction is already in InnoDB. */
    if (trx->in_depth > 1) {
      return;
    }

    trx_mutex_enter(trx);

    wait(trx);

    ut_ad((trx->in_innodb & TRX_FORCE_ROLLBACK_MASK) == 0);

    ++trx->in_innodb;

    trx_mutex_exit(trx);
  }

  /**
  Note that we are exiting InnoDB code */
  static void exit(trx_t *trx) {
    if (srv_read_only_mode) {
      return;
    }

    /* Avoid excessive mutex acquire/release */

    ut_ad(trx->in_depth > 0);

    --trx->in_depth;

    if (trx->in_depth > 0) {
      return;
    }

    trx_mutex_enter(trx);

    ut_ad((trx->in_innodb & TRX_FORCE_ROLLBACK_MASK) > 0);

    --trx->in_innodb;

    trx_mutex_exit(trx);
  }

  /*
  @return true if it is a forced rollback, asynchronously */
  static bool is_forced_rollback(const trx_t *trx) {
    ut_ad(trx_mutex_own(trx));

    return ((trx->in_innodb & TRX_FORCE_ROLLBACK)) > 0;
  }

  /**
  Wait for the asynchronous rollback to complete, if it is in progress */
  static void wait(trx_t *trx) {
    ut_ad(trx_mutex_own(trx));

    ulint loop_count = 0;
    /* start with optimistic sleep time - 20 micro seconds. */
    ulint sleep_time = 20;

    while (is_forced_rollback(trx)) {
      /* Wait for the async rollback to complete */

      trx_mutex_exit(trx);

      loop_count++;
      /* If the wait is long, don't hog the cpu. */
      if (loop_count < 100) {
        /* 20 microseconds */
        sleep_time = 20;
      } else if (loop_count < 1000) {
        /* 1 millisecond */
        sleep_time = 1000;
      } else {
        /* 100 milliseconds */
        sleep_time = 100000;
      }

      os_thread_sleep(sleep_time);

      trx_mutex_enter(trx);
    }
  }

  /**
  @return true if transaction is started */
  static bool is_started(const trx_t *trx) {
    ut_ad(trx_mutex_own(trx));

    return (trx_is_started(trx));
  }

 private:
  /**
  Transaction instance crossing the handler boundary from the Server. */
  trx_t *m_trx;
};

