#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

struct dict_index_t;

/** Compute a hash value of a prefix of a leaf page record.
@param[in]	rec		leaf page record
@param[in]	offsets		rec_get_offsets(rec)
@param[in]	n_fields	number of complete fields to fold
@param[in]	n_bytes		number of bytes to fold in the last field
@param[in]	fold		fold value of the index identifier
@param[in]	index		index where the record resides
@return the folded value */
ulint rec_fold(const rec_t *rec, const ulint *offsets, ulint n_fields,
               ulint n_bytes, ulint fold, const dict_index_t *index)
    MY_ATTRIBUTE((warn_unused_result));
