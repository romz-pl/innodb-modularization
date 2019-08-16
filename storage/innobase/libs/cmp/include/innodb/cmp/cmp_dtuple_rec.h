#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

struct dtuple_t;
struct dict_index_t;

/** Compare a data tuple to a physical record.
@see cmp_dtuple_rec_with_match
@param[in]	dtuple	data tuple
@param[in]	rec	record
@param[in]	index	index
@param[in]	offsets	rec_get_offsets(rec)
@return the comparison result of dtuple and rec
@retval 0 if dtuple is equal to rec
@retval negative if dtuple is less than rec
@retval positive if dtuple is greater than rec */
int cmp_dtuple_rec(const dtuple_t *dtuple, const rec_t *rec,
                   const dict_index_t *index, const ulint *offsets)
    MY_ATTRIBUTE((warn_unused_result));
