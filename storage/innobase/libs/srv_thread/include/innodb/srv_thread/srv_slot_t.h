#pragma once

#include <innodb/univ/univ.h>

#include <innodb/srv_thread/srv_thread_type.h>
#include <innodb/time/ib_time_t.h>
#include <innodb/sync_event/os_event_t.h>

struct que_thr_t;

#ifndef UNIV_HOTBACKUP
/** Thread slot in the thread table.  */
struct srv_slot_t {
  srv_thread_type type;   /*!< thread type: user,
                          utility etc. */
  ibool in_use;           /*!< TRUE if this slot
                          is in use */
  ibool suspended;        /*!< TRUE if the thread is
                          waiting for the event of this
                          slot */
  ib_time_t suspend_time; /*!< time when the thread was
                          suspended. Initialized by
                          lock_wait_table_reserve_slot()
                          for lock wait */
  ulong wait_timeout;     /*!< wait time that if exceeded
                          the thread will be timed out.
                          Initialized by
                          lock_wait_table_reserve_slot()
                          for lock wait */
  os_event_t event;       /*!< event used in suspending
                          the thread when it has nothing
                          to do */
  que_thr_t *thr;         /*!< suspended query thread
                          (only used for user threads) */
};
#endif /* !UNIV_HOTBACKUP */
