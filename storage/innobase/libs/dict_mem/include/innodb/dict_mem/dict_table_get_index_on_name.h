#pragma once

#include <innodb/univ/univ.h>

struct dict_table_t;
struct dict_index_t;

/** Get an index by name.
@param[in]	table		the table where to look for the index
@param[in]	name		the index name to look for
@param[in]	committed	true=search for committed,
false=search for uncommitted
@return index, NULL if does not exist */
dict_index_t *dict_table_get_index_on_name(dict_table_t *table,
                                           const char *name,
                                           bool committed = true)
    MY_ATTRIBUTE((warn_unused_result));


/** Get an index by name.
@param[in]	table		the table where to look for the index
@param[in]	name		the index name to look for
@param[in]	committed	true=search for committed,
false=search for uncommitted
@return index, NULL if does not exist */
inline const dict_index_t *dict_table_get_index_on_name(
    const dict_table_t *table, const char *name, bool committed = true) {
  return (dict_table_get_index_on_name(const_cast<dict_table_t *>(table), name,
                                       committed));
}
