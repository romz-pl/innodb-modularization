#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

/** TRUE if the record is a user record on the page.
 @return true if a user record */
ibool page_rec_is_user_rec(const rec_t *rec) /*!< in: record */
    MY_ATTRIBUTE((warn_unused_result));
