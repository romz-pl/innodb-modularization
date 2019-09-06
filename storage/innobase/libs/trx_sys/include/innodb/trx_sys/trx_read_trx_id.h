#pragma once

#include <innodb/univ/univ.h>

/** Reads a trx id from an index page. In case that the id size changes in
 some future version, this function should be used instead of
 mach_read_...
 @return id */
trx_id_t trx_read_trx_id(
    const byte *ptr); /*!< in: pointer to memory from where to read */
