#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_array/sync_cell_t.h>

/* NOTE: It is allowed for a thread to wait for an event allocated for
the array without owning the protecting mutex (depending on the case:
OS or database mutex), but all changes (set or reset) to the state of
the event must be made while owning the mutex. */

/** Synchronization array */
struct sync_array_t {
  /** Constructor
  Creates a synchronization wait array. It is protected by a mutex
  which is automatically reserved when the functions operating on it
  are called.
  @param[in]	num_cells	Number of cells to create */
  sync_array_t(ulint num_cells) UNIV_NOTHROW;

  /** Destructor */
  ~sync_array_t() UNIV_NOTHROW;

  ulint n_reserved;      /*!< number of currently reserved
                         cells in the wait array */
  ulint n_cells;         /*!< number of cells in the
                         wait array */
  sync_cell_t *cells;    /*!< pointer to wait array */
  SysMutex mutex;        /*!< System mutex protecting the
                         data structure.  As this data
                         structure is used in constructing
                         the database mutex, to prevent
                         infinite recursion in implementation,
                         we fall back to an OS mutex. */
  ulint res_count;       /*!< count of cell reservations
                         since creation of the array */
  ulint next_free_slot;  /*!< the next free cell in the array */
  ulint first_free_slot; /*!< the last slot that was freed */
};
