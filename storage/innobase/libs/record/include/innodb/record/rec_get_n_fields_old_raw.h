#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/rec_t.h>

/** The following function is used to get the number of fields
 in an old-style record, which is stored in the rec
 @return number of data fields */
MY_ATTRIBUTE((warn_unused_result)) uint16_t
    rec_get_n_fields_old_raw(const rec_t *rec); /*!< in: physical record */
