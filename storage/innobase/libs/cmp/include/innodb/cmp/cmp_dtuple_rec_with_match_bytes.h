#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

struct dtuple_t;
struct dict_index_t;

/** Compare a data tuple to a physical record.
@param[in]	dtuple		data tuple
@param[in]	rec		B-tree or R-tree index record
@param[in]	index		index tree
@param[in]	offsets		rec_get_offsets(rec)
@param[in,out]	matched_fields	number of completely matched fields
@param[in,out]	matched_bytes	number of matched bytes in the first
field that is not matched
@return the comparison result of dtuple and rec
@retval 0 if dtuple is equal to rec
@retval negative if dtuple is less than rec
@retval positive if dtuple is greater than rec */
int cmp_dtuple_rec_with_match_bytes(const dtuple_t *dtuple, const rec_t *rec,
                                    const dict_index_t *index,
                                    const ulint *offsets, ulint *matched_fields,
                                    ulint *matched_bytes)
    MY_ATTRIBUTE((warn_unused_result));
