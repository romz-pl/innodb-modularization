#pragma once

#include <innodb/univ/univ.h>

struct dfield_t;

/** Tests if two data fields are equal.
 If len==0, tests the data length and content for equality.
 If len>0, tests the first len bytes of the content for equality.
 @return true if both fields are NULL or if they are equal */
ibool dfield_datas_are_binary_equal(
    const dfield_t *field1, /*!< in: field */
    const dfield_t *field2, /*!< in: field */
    ulint len)              /*!< in: maximum prefix to compare,
                            or 0 to compare the whole field length */
    MY_ATTRIBUTE((warn_unused_result));
