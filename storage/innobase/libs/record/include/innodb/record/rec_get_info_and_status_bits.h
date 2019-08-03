#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/rec_t.h>

/** The following function is used to retrieve the info and status
 bits of a record.  (Only compact records have status bits.)
 @return info bits */
ulint rec_get_info_and_status_bits(
    const rec_t *rec, /*!< in: physical record */
    ulint comp)       /*!< in: nonzero=compact page format */
    MY_ATTRIBUTE((warn_unused_result));
