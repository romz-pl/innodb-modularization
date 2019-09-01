#pragma once

#include <innodb/univ/univ.h>

#include <innodb/lock_sys/lock_sys.h>

/** Test if lock_sys->mutex can be acquired without waiting. */
#define lock_mutex_enter_nowait() (lock_sys->mutex.trylock(__FILE__, __LINE__))
