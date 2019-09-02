#pragma once

#include <innodb/univ/univ.h>
#include <innodb/univ/page_no_t.h>

struct lock_t;

/** Determines if there are explicit record locks on a page.
 @return an explicit record lock on the page, or NULL if there are none */
lock_t *lock_rec_expl_exist_on_page(space_id_t space,  /*!< in: space id */
                                    page_no_t page_no) /*!< in: page number */
    MY_ATTRIBUTE((warn_unused_result));
