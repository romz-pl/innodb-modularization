#pragma once

#include <innodb/univ/univ.h>

struct dfield_t;

/** Gets length of field data.
 @return length of data; UNIV_SQL_NULL if SQL null data */
ulint dfield_get_len(const dfield_t *field) /*!< in: field */
    MY_ATTRIBUTE((warn_unused_result));
