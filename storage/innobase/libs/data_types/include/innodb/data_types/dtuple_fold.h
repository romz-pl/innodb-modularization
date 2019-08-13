#pragma once

#include <innodb/univ/univ.h>

struct dtuple_t;

/** Compute a hash value of a prefix of an index record.
@param[in]	tuple		index record
@param[in]	n_fields	number of fields to include
@param[in]	n_bytes		number of bytes to fold in the last field
@param[in]	fold		fold value of the index identifier
@return the folded value */
ulint dtuple_fold(const dtuple_t *tuple, ulint n_fields, ulint n_bytes,
                  ulint fold) MY_ATTRIBUTE((warn_unused_result));
