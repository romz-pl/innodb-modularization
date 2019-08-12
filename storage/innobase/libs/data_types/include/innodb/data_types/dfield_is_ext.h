#pragma once

#include <innodb/univ/univ.h>

struct dfield_t;

/** Determines if a field is externally stored
 @return nonzero if externally stored */
ulint dfield_is_ext(const dfield_t *field) /*!< in: field */
    MY_ATTRIBUTE((warn_unused_result));
