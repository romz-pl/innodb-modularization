#pragma once

#include <innodb/univ/univ.h>

/** Wakes up simulated aio i/o-handler threads if they have something to do. */
void os_aio_simulated_wake_handler_threads();
