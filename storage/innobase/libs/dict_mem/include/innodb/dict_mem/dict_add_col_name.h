#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>

/** Append 'name' to 'col_names'.  @see dict_table_t::col_names
 @return new column names array */
const char *dict_add_col_name(
    const char *col_names, /*!< in: existing column names, or
                           NULL */
    ulint cols,            /*!< in: number of existing columns */
    const char *name,      /*!< in: new column name */
    mem_heap_t *heap);     /*!< in: heap */
