#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_sys.h>

/** Check whether the table is DDTableBuffer. See class DDTableBuffer
@param[in]	table	table to check
@return true if this is a DDTableBuffer table. */
UNIV_INLINE
bool dict_table_is_table_buffer(const dict_table_t *table) {
  return (table == dict_sys->dynamic_metadata);
}
#endif /* !UNIV_HOTBACKUP */
