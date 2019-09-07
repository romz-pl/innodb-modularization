#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_purge/trx_purge_t.h>

/** The global data structure coordinating a purge */
extern trx_purge_t *purge_sys;
