#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/rec_t.h>

/** TRUE if the record is the infimum record on a page.
 @return true if the infimum record */
ibool page_rec_is_infimum(const rec_t *rec) /*!< in: record */
    MY_ATTRIBUTE((warn_unused_result));
