#pragma once

#include <innodb/univ/univ.h>

struct dfield_t;

/** Checks that a data field is typed. Asserts an error if not.
 @return true if ok */
ibool dfield_check_typed(const dfield_t *field) /*!< in: data field */
    MY_ATTRIBUTE((warn_unused_result));
