#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

/** The following function is used to test whether the data offsets in the
 record are stored in one-byte or two-byte format.
 @return true if 1-byte form */
MY_ATTRIBUTE((warn_unused_result)) ibool
    rec_get_1byte_offs_flag(const rec_t *rec); /*!< in: physical record */
