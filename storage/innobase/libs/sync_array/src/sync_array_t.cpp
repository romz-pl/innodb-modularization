#include <innodb/sync_array/sync_array_t.h>

#include <innodb/sync_mutex/mutex_create.h>
#include <innodb/sync_mutex/mutex_free.h>
#include <innodb/allocator/UT_DELETE_ARRAY.h>
#include <innodb/allocator/UT_NEW_ARRAY_NOKEY.h>

/** Constructor
Creates a synchronization wait array. It is protected by a mutex
which is automatically reserved when the functions operating on it
are called.
@param[in]	num_cells		Number of cells to create */
sync_array_t::sync_array_t(ulint num_cells) UNIV_NOTHROW : n_reserved(),
                                                           n_cells(),
                                                           cells(),
                                                           mutex(),
                                                           res_count(),
                                                           next_free_slot(),
                                                           first_free_slot() {
  ut_a(num_cells > 0);

  cells = UT_NEW_ARRAY_NOKEY(sync_cell_t, num_cells);

  ulint sz = sizeof(sync_cell_t) * num_cells;

  memset(cells, 0x0, sz);

  n_cells = num_cells;

  first_free_slot = ULINT_UNDEFINED;

  /* Then create the mutex to protect the wait array */
  mutex_create(LATCH_ID_SYNC_ARRAY_MUTEX, &mutex);
}

/** Destructor */
sync_array_t::~sync_array_t() UNIV_NOTHROW {
  ut_a(n_reserved == 0);

  ut_d(sync_array_validate(this));

  /* Release the mutex protecting the wait array */

  mutex_free(&mutex);

  UT_DELETE_ARRAY(cells);
}
