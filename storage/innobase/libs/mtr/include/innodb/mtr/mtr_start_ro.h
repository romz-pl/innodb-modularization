#pragma once

#include <innodb/univ/univ.h>

#include <innodb/mtr/mtr_t.h>

/** Start an asynchronous read-only mini-transaction */
#define mtr_start_ro(m) (m)->start(true, true)
