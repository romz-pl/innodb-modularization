#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/rec_t.h>

/** Sets the field end info for the nth field if the record is stored in the
 2-byte format. */
void rec_2_set_field_end_info(rec_t *rec, /*!< in: record */
                              ulint n,    /*!< in: field index */
                              ulint info); /*!< in: value to set */
