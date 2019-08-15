#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>

/************************************************************************
Check if the table has an FTS index. */
UNIV_INLINE
ibool dict_table_has_fts_index(
    /* out: TRUE if table has an FTS index */
    dict_table_t *table) /* in: table */
{
  ut_ad(table);

  return (DICT_TF2_FLAG_IS_SET(table, DICT_TF2_FTS));
}
