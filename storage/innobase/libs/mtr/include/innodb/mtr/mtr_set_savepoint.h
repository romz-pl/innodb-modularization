#pragma once

#include <innodb/univ/univ.h>

#include <innodb/mtr/mtr_t.h>

/** Set and return a savepoint in mtr.
@return	savepoint */
#define mtr_set_savepoint(m) (m)->get_savepoint()
