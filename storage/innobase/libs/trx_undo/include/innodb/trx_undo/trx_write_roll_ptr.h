#pragma once

#include <innodb/univ/univ.h>

/** Writes a roll ptr to an index page. In case that the size changes in
some future version, this function should be used instead of
mach_write_...
@param[in]	ptr		pointer to memory where written
@param[in]	roll_ptr	roll ptr */
void trx_write_roll_ptr(byte *ptr, roll_ptr_t roll_ptr);
