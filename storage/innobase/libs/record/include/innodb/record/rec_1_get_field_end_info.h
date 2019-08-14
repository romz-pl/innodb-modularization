#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

/** Returns the offset of nth field end if the record is stored in the 1-byte
 offsets form. If the field is SQL null, the flag is ORed in the returned
 value.
 @return offset of the start of the field, SQL null flag ORed */
MY_ATTRIBUTE((warn_unused_result)) ulint
    rec_1_get_field_end_info(const rec_t *rec, /*!< in: record */
                             ulint n);          /*!< in: field index */
