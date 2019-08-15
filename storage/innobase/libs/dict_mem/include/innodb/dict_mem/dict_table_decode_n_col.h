#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/flags.h>

/** decode number of virtual and non-virtual columns in one 4 bytes value.
@param[in]      encoded encoded value
@param[in,out]     n_col   number of non-virtual column
@param[in,out]     n_v_col number of virtual column */
UNIV_INLINE
void dict_table_decode_n_col(uint32_t encoded, uint32_t *n_col,
                             uint32_t *n_v_col) {
  uint32_t num = encoded & ~DICT_N_COLS_COMPACT;
  *n_v_col = num >> 16;
  *n_col = num & 0xFFFF;
}
