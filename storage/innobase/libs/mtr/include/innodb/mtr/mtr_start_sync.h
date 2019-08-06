#pragma once

#include <innodb/univ/univ.h>

#include <innodb/mtr/mtr_t.h>


/** Start a synchronous mini-transaction */
#define mtr_start_sync(m) (m)->start(true)
