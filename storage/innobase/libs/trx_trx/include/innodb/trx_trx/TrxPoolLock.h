#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_mutex/ib_mutex_t.h>
#include <innodb/sync_mutex/mutex_create.h>
#include <innodb/sync_mutex/mutex_exit.h>
#include <innodb/sync_mutex/mutex_free.h>
#include <innodb/sync_mutex/mutex_enter.h>

/** The lock strategy for TrxPool */
struct TrxPoolLock {
  TrxPoolLock() {}

  /** Create the mutex */
  void create() { mutex_create(LATCH_ID_TRX_POOL, &m_mutex); }

  /** Acquire the mutex */
  void enter() { mutex_enter(&m_mutex); }

  /** Release the mutex */
  void exit() { mutex_exit(&m_mutex); }

  /** Free the mutex */
  void destroy() { mutex_free(&m_mutex); }

  /** Mutex to use */
  ib_mutex_t m_mutex;
};
