#pragma once

#include <innodb/univ/univ.h>

/** Get the column number for a virtual column (the column position in
original table), stored in the "POS" field of Sys_columns
@param[in]	pos		virtual column position
@return column position in original table */
UNIV_INLINE
ulint dict_get_v_col_mysql_pos(ulint pos) {
    return (pos & 0xFFFF);
}
