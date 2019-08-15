#pragma once

#include <innodb/univ/univ.h>

/** Get a virtual column sequence (the "nth" virtual column) for a
virtual column, stord in the "POS" field of Sys_columns
@param[in]	pos		virtual column position
@return virtual column sequence */
UNIV_INLINE
ulint dict_get_v_col_pos(ulint pos) {
    return ((pos >> 16) - 1);
}
