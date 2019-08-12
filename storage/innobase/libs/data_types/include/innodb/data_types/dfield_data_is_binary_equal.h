#pragma once

#include <innodb/univ/univ.h>

struct dfield_t;

/** Tests if dfield data length and content is equal to the given.
 @return true if equal */
ibool dfield_data_is_binary_equal(
    const dfield_t *field, /*!< in: field */
    ulint len,             /*!< in: data length or UNIV_SQL_NULL */
    const byte *data)      /*!< in: data */
    MY_ATTRIBUTE((warn_unused_result));
