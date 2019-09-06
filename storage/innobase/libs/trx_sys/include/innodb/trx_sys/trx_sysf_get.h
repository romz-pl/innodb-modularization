#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_types/trx_sysf_t.h>

struct mtr_t;

/** Gets a pointer to the transaction system file copy and x-locks its page.
 @return pointer to system file copy, page x-locked */
trx_sysf_t *trx_sysf_get(mtr_t *mtr); /*!< in: mtr */
