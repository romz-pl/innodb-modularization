#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

/** Returns the max trx id field value. */
trx_id_t page_get_max_trx_id(const page_t *page); /*!< in: page */
