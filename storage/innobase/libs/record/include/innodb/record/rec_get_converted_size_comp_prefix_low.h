#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

struct dict_index_t;
struct dfield_t;
struct dtuple_t;

/** Determines the size of a data tuple prefix in ROW_FORMAT=COMPACT.
 @return total size */
UNIV_INLINE MY_ATTRIBUTE((warn_unused_result)) ulint
    rec_get_converted_size_comp_prefix_low(
        const dict_index_t *index, /*!< in: record descriptor;
                                   dict_table_is_comp() is
                                   assumed to hold, even if
                                   it does not */
        const dfield_t *fields,    /*!< in: array of data fields */
        ulint n_fields,            /*!< in: number of data fields */
        const dtuple_t *v_entry,   /*!< in: dtuple contains virtual column
                                   data */
        ulint *extra,              /*!< out: extra size */
        ulint *status,             /*!< in: status bits of the record,
                                   can be nullptr if unnecessary */
        bool temp);                 /*!< in: whether this is a
                                   temporary file record */
