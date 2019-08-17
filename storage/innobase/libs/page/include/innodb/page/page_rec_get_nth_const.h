#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>
#include <innodb/univ/rec_t.h>

/** Returns the nth record of the record list.
 This is the inverse function of page_rec_get_n_recs_before().
 @return nth record */
const rec_t *page_rec_get_nth_const(const page_t *page, /*!< in: page */
                                    ulint nth)          /*!< in: nth record */
    MY_ATTRIBUTE((warn_unused_result));
