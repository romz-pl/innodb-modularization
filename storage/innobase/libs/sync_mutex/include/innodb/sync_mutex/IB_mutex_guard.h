#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_mutex/ib_mutex_t.h>
#include <innodb/sync_mutex/mutex_enter.h>
#include <innodb/sync_mutex/mutex_exit.h>

#ifndef UNIV_LIBRARY
#ifndef UNIV_HOTBACKUP

/* RAII guard for ib mutex */
struct IB_mutex_guard {
  /** Constructor to acquire mutex
  @param[in]   in_mutex        input mutex */
  explicit IB_mutex_guard(ib_mutex_t *in_mutex) : m_mutex(in_mutex) {
    mutex_enter(in_mutex);
  }

  /** Destructor to release mutex */
  ~IB_mutex_guard() { mutex_exit(m_mutex); }

  /** Disable copy construction */
  IB_mutex_guard(IB_mutex_guard const &) = delete;

  /** Disable assignment */
  IB_mutex_guard &operator=(IB_mutex_guard const &) = delete;

 private:
  /** Current mutex for RAII */
  ib_mutex_t *m_mutex;
};

#endif
#endif
