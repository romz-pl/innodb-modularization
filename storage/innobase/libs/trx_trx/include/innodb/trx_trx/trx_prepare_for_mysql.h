#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>

struct trx_t;

/**
Does the transaction prepare for MySQL.
@param[in, out] trx		Transaction instance to prepare */

dberr_t trx_prepare_for_mysql(trx_t *trx);
