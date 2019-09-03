#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_sys/trx_sys.h>

/** Test if trx_sys->mutex is owned. */
#define trx_sys_mutex_own() (trx_sys->mutex.is_owned())
