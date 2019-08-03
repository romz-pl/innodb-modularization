#pragma once

#include <innodb/univ/univ.h>

/** TRUE if the record is the supremum record on a page.
 @return true if the supremum record */
ibool page_rec_is_supremum_low(ulint offset) /*!< in: record offset on page */
    MY_ATTRIBUTE((const));
