#pragma once

#include <innodb/univ/univ.h>

/** TRUE if the record is a user record on the page.
 @return true if a user record */
ibool page_rec_is_user_rec_low(ulint offset) /*!< in: record offset on page */
    MY_ATTRIBUTE((const));
