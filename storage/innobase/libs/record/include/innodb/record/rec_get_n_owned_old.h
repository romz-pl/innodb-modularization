#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

/** The following function is used to get the number of records owned by the
 previous directory record.
 @return number of owned records */
ulint rec_get_n_owned_old(
    const rec_t *rec) /*!< in: old-style physical record */
    MY_ATTRIBUTE((warn_unused_result));
