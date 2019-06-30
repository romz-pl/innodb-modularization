#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_array/sync_object_t.h>

/** A cell where an individual thread may wait suspended until a resource
is released. The suspending is implemented using an operating system
event semaphore. */

struct sync_cell_t {
  sync_object_t latch;      /*!< pointer to the object the
                            thread is waiting for; if NULL
                            the cell is free for use */
  ulint request_type;       /*!< lock type requested on the
                            object */
  const char *file;         /*!< in debug version file where
                            requested */
  ulint line;               /*!< in debug version line where
                            requested */
  os_thread_id_t thread_id; /*!< thread id of this waiting
                            thread */
  bool waiting;             /*!< TRUE if the thread has already
                            called sync_array_event_wait
                            on this cell */
  int64_t signal_count;     /*!< We capture the signal_count
                            of the latch when we
                            reset the event. This value is
                            then passed on to os_event_wait
                            and we wait only if the event
                            has not been signalled in the
                            period between the reset and
                            wait call. */
  time_t reservation_time;  /*!< time when the thread reserved
                           the wait cell */
};
