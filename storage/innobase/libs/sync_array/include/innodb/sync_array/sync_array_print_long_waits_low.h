#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_array/sync_array_t.h>

/** Prints warnings of long semaphore waits to stderr.
 @return true if fatal semaphore wait threshold was exceeded */
bool sync_array_print_long_waits_low(
    sync_array_t *arr,      /*!< in: sync array instance */
    os_thread_id_t *waiter, /*!< out: longest waiting thread */
    const void **sema,      /*!< out: longest-waited-for semaphore */
    ibool *noticed);         /*!< out: TRUE if long wait noticed */
