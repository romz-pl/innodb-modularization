#pragma once

#include <innodb/univ/univ.h>

struct dict_col_t;

/** This function populates a dict_col_t memory structure with
 supplied information. */
void dict_mem_fill_column_struct(
    dict_col_t *column, /*!< out: column struct to be
                        filled */
    ulint col_pos,      /*!< in: column position */
    ulint mtype,        /*!< in: main data type */
    ulint prtype,       /*!< in: precise type */
    ulint col_len);     /*!< in: column length */
