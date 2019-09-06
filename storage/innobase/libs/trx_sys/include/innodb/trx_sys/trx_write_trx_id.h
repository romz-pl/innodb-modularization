#pragma once

#include <innodb/univ/univ.h>

/** Writes a trx id to an index page. In case that the id size changes in some
future version, this function should be used instead of mach_write_...
@param[in]	ptr	pointer to memory where written
@param[in]	id	id */
void trx_write_trx_id(byte *ptr, trx_id_t id);
