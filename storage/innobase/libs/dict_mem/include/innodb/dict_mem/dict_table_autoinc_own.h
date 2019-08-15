#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_DEBUG
/** Check if the current thread owns the autoinc_mutex of a given table.
@param[in]	table	the autoinc_mutex belongs to this table
@return true, if the current thread owns the autoinc_mutex, false otherwise.*/
inline bool dict_table_autoinc_own(const dict_table_t *table) {
  return (mutex_own(table->autoinc_mutex));
}
#endif /* UNIV_DEBUG */
