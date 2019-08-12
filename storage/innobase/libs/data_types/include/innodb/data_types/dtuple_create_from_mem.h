#pragma once

#include <innodb/univ/univ.h>

struct dtuple_t;

/** Creates a data tuple from an already allocated chunk of memory.
 The size of the chunk must be at least DTUPLE_EST_ALLOC(n_fields).
 The default value for number of fields used in record comparisons
 for this tuple is n_fields.
 @param[in,out]	buf		buffer to use
 @param[in]	buf_size	buffer size
 @param[in]	n_fields	number of field
 @param[in]	n_v_fields	number of fields on virtual columns
 @return created tuple (inside buf) */
dtuple_t *dtuple_create_from_mem(void *buf, ulint buf_size, ulint n_fields,
                                 ulint n_v_fields)
    MY_ATTRIBUTE((warn_unused_result));
