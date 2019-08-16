#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

struct dict_index_t;



/** The following function determines the offsets to each field in the
 record.	 The offsets are written to a previously allocated array of
 ulint, where rec_offs_n_fields(offsets) has been initialized to the
 number of fields in the record.	 The rest of the array will be
 initialized by this function.  rec_offs_base(offsets)[0] will be set
 to the extra size (if REC_OFFS_COMPACT is set, the record is in the
 new format; if REC_OFFS_EXTERNAL is set, the record contains externally
 stored columns), and rec_offs_base(offsets)[1..n_fields] will be set to
 offsets past the end of fields 0..n_fields, or to the beginning of
 fields 1..n_fields+1.  When the high-order bit of the offset at [i+1]
 is set (REC_OFFS_SQL_NULL), the field i is NULL.  When the second
 high-order bit of the offset at [i+1] is set (REC_OFFS_EXTERNAL), the
 field i is being stored externally. */
void rec_init_offsets(const rec_t *rec,          /*!< in: physical record */
                      const dict_index_t *index, /*!< in: record descriptor */
                      ulint *offsets);           /*!< in/out: array of offsets;
                                                in: n=rec_offs_n_fields(offsets) */


