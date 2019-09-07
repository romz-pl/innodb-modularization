#pragma once

#include <innodb/univ/univ.h>

/** Read from an undo log record a non-virtual column value.
@param[in,out]	ptr		pointer to remaining part of the undo record
@param[in,out]	field		stored field
@param[in,out]	len		length of the field, or UNIV_SQL_NULL
@param[in,out]	orig_len	original length of the locally stored part
of an externally stored column, or 0
@return remaining part of undo log record after reading these values */
byte *trx_undo_rec_get_col_val(const byte *ptr, const byte **field, ulint *len,
                               ulint *orig_len);
