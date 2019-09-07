#pragma once

#include <innodb/univ/univ.h>

/** Reads a roll ptr from an index page. In case that the roll ptr size
 changes in some future version, this function should be used instead of
 mach_read_...
 @return roll ptr */
roll_ptr_t trx_read_roll_ptr(
    const byte *ptr); /*!< in: pointer to memory from where to read */
