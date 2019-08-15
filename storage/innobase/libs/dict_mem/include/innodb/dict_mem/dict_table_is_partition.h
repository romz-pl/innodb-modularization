#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>

/** Check whether the dict_table_t is a partition.
A partitioned table on the SQL level is composed of InnoDB tables,
where each InnoDB table is a [sub]partition including its secondary indexes
which belongs to the partition.
@param[in]	table	Table to check.
@return true if the dict_table_t is a partition else false. */
UNIV_INLINE
bool dict_table_is_partition(const dict_table_t *table) {
  /* Check both P and p on all platforms in case it was moved to/from
  WIN. */
  return (strstr(table->name.m_name, "#p#") ||
          strstr(table->name.m_name, "#P#"));
}
