#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>
#include <innodb/disk/page_t.h>

/** true if the record is the first user record on a page.
 @return true if the first user record */
bool page_rec_is_first(const rec_t *rec,   /*!< in: record */
                       const page_t *page) /*!< in: page */
    MY_ATTRIBUTE((warn_unused_result));
