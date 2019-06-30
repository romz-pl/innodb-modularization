#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_array/sync_cell_t.h>

/** Returns the event that the thread owning the cell waits for. */
os_event_t sync_cell_get_event(
    sync_cell_t *cell); /*!< in: non-empty sync array cell */
