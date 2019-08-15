#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_table_has_autoinc_col.h>
#include <innodb/sync_mutex/mutex_own.h>

/** Update the persisted autoinc counter to specified one, we should hold
autoinc_persisted_mutex.
@param[in,out]	table	table
@param[in]	counter	set autoinc_persisted to this value */
UNIV_INLINE
void dict_table_autoinc_persisted_update(dict_table_t *table,
                                         ib_uint64_t autoinc) {
#ifndef UNIV_HOTBACKUP
  ut_ad(dict_table_has_autoinc_col(table));
  ut_ad(mutex_own(table->autoinc_persisted_mutex));
#endif /* !UNIV_HOTBACKUP */

  table->autoinc_persisted = autoinc;
}
