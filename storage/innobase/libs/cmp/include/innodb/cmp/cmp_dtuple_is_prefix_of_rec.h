#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

struct dtuple_t;
struct dict_index_t;

/** Check if a dtuple is a prefix of a record.
@param[in]	dtuple	data tuple
@param[in]	rec	B-tree record
@param[in]	index	B-tree index
@param[in]	offsets	rec_get_offsets(rec)
@return true if prefix */
ibool cmp_dtuple_is_prefix_of_rec(const dtuple_t *dtuple, const rec_t *rec,
                                  const dict_index_t *index,
                                  const ulint *offsets)
    MY_ATTRIBUTE((warn_unused_result));
