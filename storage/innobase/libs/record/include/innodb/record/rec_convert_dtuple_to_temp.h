#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

struct dict_index_t;
struct dfield_t;
struct dtuple_t;

/** Builds a temporary file record out of a data tuple.
 @see rec_init_offsets_temp() */
void rec_convert_dtuple_to_temp(
    rec_t *rec,                /*!< out: record */
    const dict_index_t *index, /*!< in: record descriptor */
    const dfield_t *fields,    /*!< in: array of data fields */
    ulint n_fields,            /*!< in: number of fields */
    const dtuple_t *v_entry);  /*!< in: dtuple contains
                               virtual column data */
