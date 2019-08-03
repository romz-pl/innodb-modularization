#pragma once

#include <innodb/univ/univ.h>

/** TRUE if the record is the infimum record on a page.
 @return true if the infimum record */
ibool page_rec_is_infimum_low(ulint offset) /*!< in: record offset on page */
    MY_ATTRIBUTE((const));
