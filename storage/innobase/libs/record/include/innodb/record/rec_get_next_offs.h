#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

/** The following function is used to get the offset of the
 next chained record on the same page.
 @return the page offset of the next chained record, or 0 if none */
ulint rec_get_next_offs(const rec_t *rec, /*!< in: physical record */
                        ulint comp) /*!< in: nonzero=compact page format */
    MY_ATTRIBUTE((warn_unused_result));
