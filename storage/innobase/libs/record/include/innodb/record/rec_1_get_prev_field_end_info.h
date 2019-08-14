#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

/** Returns the offset of n - 1th field end if the record is stored in the
 1-byte offsets form. If the field is SQL null, the flag is ORed in the returned
 value. This function and the 2-byte counterpart are defined here because the
 C-compiler was not able to sum negative and positive constant offsets, and
 warned of constant arithmetic overflow within the compiler.
 @return offset of the start of the PREVIOUS field, SQL null flag ORed */
ulint rec_1_get_prev_field_end_info(const rec_t *rec, /*!< in: record */
                                    ulint n);          /*!< in: field index */
