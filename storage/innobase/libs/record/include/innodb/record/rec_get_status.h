#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/rec_t.h>

/** The following function retrieves the status bits of a new-style record.
 @return status bits */
MY_ATTRIBUTE((warn_unused_result)) ulint
    rec_get_status(const rec_t *rec); /*!< in: physical record */
