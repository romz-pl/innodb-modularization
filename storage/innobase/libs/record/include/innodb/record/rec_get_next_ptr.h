#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

/** The following function is used to get the pointer of the next chained record
 on the same page.
 @return pointer to the next chained record, or NULL if none */
rec_t *rec_get_next_ptr(rec_t *rec, /*!< in: physical record */
                        ulint comp) /*!< in: nonzero=compact page format */
    MY_ATTRIBUTE((warn_unused_result));
