#pragma once

#include <innodb/univ/univ.h>

#include <innodb/lock_types/lock_mode.h>

/** Calculates if lock mode 1 is compatible with lock mode 2.
@param[in]	mode1	lock mode
@param[in]	mode2	lock mode
@return nonzero if mode1 compatible with mode2 */
ulint lock_mode_compatible(enum lock_mode mode1, enum lock_mode mode2);
