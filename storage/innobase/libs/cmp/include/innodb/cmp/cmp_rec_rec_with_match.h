#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

struct dict_index_t;

/** Compare two B-tree records.
@param[in] rec1 B-tree record
@param[in] rec2 B-tree record
@param[in] offsets1 rec_get_offsets(rec1, index)
@param[in] offsets2 rec_get_offsets(rec2, index)
@param[in] index B-tree index
@param[in] nulls_unequal true if this is for index cardinality
statistics estimation, and innodb_stats_method=nulls_unequal
or innodb_stats_method=nulls_ignored
@param[out] matched_fields number of completely matched fields
within the first field not completely matched
@return the comparison result
@retval 0 if rec1 is equal to rec2
@retval negative if rec1 is less than rec2
@retval positive if rec2 is greater than rec2 */
int cmp_rec_rec_with_match(const rec_t *rec1, const rec_t *rec2,
                           const ulint *offsets1, const ulint *offsets2,
                           const dict_index_t *index, bool nulls_unequal,
                           ulint *matched_fields);
