#pragma once

#include <innodb/univ/univ.h>

struct dfield_t;

/** Determines if a field is SQL NULL
 @return nonzero if SQL null data */
ulint dfield_is_null(const dfield_t *field) /*!< in: field */
    MY_ATTRIBUTE((warn_unused_result));
