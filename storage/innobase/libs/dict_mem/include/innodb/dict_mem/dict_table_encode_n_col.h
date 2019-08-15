#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_table_t.h>

/** Encode the number of columns and number of virtual columns in a
4 bytes value. We could do this because the number of columns in
InnoDB is limited to 1017
@param[in]      n_col   number of non-virtual column
@param[in]      n_v_col number of virtual column
@return encoded value */
UNIV_INLINE
ulint dict_table_encode_n_col(ulint n_col, ulint n_v_col) {
  return (n_col + (n_v_col << 16));
}
