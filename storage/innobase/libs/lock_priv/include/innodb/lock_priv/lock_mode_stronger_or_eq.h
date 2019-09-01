#pragma once

#include <innodb/univ/univ.h>

#include <innodb/lock_types/lock_mode.h>

/** Calculates if lock mode 1 is stronger or equal to lock mode 2.
@param[in]	mode1	lock mode
@param[in]	mode2	lock mode
@return nonzero if mode1 stronger or equal to mode2 */
ulint lock_mode_stronger_or_eq(enum lock_mode mode1, enum lock_mode mode2);
