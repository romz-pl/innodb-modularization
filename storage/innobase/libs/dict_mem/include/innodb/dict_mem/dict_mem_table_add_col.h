#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>

struct dict_table_t;

/** Adds a column definition to a table. */
void dict_mem_table_add_col(
    dict_table_t *table, /*!< in: table */
    mem_heap_t *heap,    /*!< in: temporary memory heap, or NULL */
    const char *name,    /*!< in: column name, or NULL */
    ulint mtype,         /*!< in: main datatype */
    ulint prtype,        /*!< in: precise type */
    ulint len);          /*!< in: precision */
