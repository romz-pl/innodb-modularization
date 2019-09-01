#pragma once

#include <innodb/univ/univ.h>

#include <innodb/lock_sys/lock_sys.h>

/** Test if lock_sys->mutex is owned. */
#define lock_mutex_own() (lock_sys->mutex.is_owned())
