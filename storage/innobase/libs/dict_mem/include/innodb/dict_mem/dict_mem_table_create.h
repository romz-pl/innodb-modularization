#pragma once

#include <innodb/univ/univ.h>

struct dict_table_t;

/** Creates a table memory object.
 @return own: table object */
dict_table_t *dict_mem_table_create(
    const char *name, /*!< in: table name */
    space_id_t space, /*!< in: space where the clustered index
                      of the table is placed */
    ulint n_cols,     /*!< in: total number of columns
                      including virtual and non-virtual
                      columns */
    ulint n_v_cols,   /*!< in: number of virtual columns */
    uint32_t flags,   /*!< in: table flags */
    uint32_t flags2); /*!< in: table flags2 */
