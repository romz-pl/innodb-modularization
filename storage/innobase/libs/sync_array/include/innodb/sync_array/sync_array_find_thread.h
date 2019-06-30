#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_DEBUG

#include <innodb/sync_array/sync_cell_t.h>

/** Looks for a cell with the given thread id.
 @return pointer to cell or NULL if not found */
sync_cell_t *sync_array_find_thread(
    sync_array_t *arr,     /*!< in: wait array */
    os_thread_id_t thread); /*!< in: thread id */

#endif
