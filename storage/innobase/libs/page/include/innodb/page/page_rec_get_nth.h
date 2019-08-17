#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>
#include <innodb/disk/page_t.h>


/** Returns the nth record of the record list.
This is the inverse function of page_rec_get_n_recs_before().
@param[in]	page	page
@param[in]	nth	nth record
@return nth record */
rec_t *page_rec_get_nth(page_t *page, ulint nth)
    MY_ATTRIBUTE((warn_unused_result));
