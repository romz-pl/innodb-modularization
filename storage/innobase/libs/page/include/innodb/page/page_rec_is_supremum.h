#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

/** TRUE if the record is the supremum record on a page.
 @return true if the supremum record */
ibool page_rec_is_supremum(const rec_t *rec) /*!< in: record */
    MY_ATTRIBUTE((warn_unused_result));
