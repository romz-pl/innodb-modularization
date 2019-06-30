#pragma once

#include <innodb/univ/univ.h>

#include <innodb/thread/os_thread_id_t.h>

/** Prints warnings of long semaphore waits to stderr.
 @return true if fatal semaphore wait threshold was exceeded */
ibool sync_array_print_long_waits(
    os_thread_id_t *waiter, /*!< out: longest waiting thread */
    const void **sema);      /*!< out: longest-waited-for semaphore */
