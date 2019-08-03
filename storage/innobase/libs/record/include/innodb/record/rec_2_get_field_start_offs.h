#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/rec_t.h>

/** Returns the offset of nth field start if the record is stored in the 2-byte
 offsets form.
 @return offset of the start of the field */
ulint rec_2_get_field_start_offs(const rec_t *rec, /*!< in: record */
                                 ulint n);          /*!< in: field index */
