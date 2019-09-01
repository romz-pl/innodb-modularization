#pragma once

#include <innodb/univ/univ.h>

#include <innodb/lock_sys/lock_sys.h>

/** Test if lock_sys->wait_mutex is owned. */
#define lock_wait_mutex_own() (lock_sys->wait_mutex.is_owned())
