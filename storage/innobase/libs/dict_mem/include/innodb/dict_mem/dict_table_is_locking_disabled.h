#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_table_is_table_buffer.h>

/** Check whether locking is disabled for this table.
Currently this is done for intrinsic table as their visibility is limited
to the connection and the DDTableBuffer as it's protected by
dict_persist->mutex.

@param[in]	table	table to check
@return true if locking is disabled. */
UNIV_INLINE
bool dict_table_is_locking_disabled(const dict_table_t *table) {
  return (table->is_intrinsic() || dict_table_is_table_buffer(table));
}

#endif /* !UNIV_HOTBACKUP */
