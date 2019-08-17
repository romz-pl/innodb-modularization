#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>
#include <innodb/disk/page_t.h>


#ifndef UNIV_HOTBACKUP
/** Returns the middle record of the records on the page. If there is an
 even number of records in the list, returns the first record of the
 upper half-list.
 @return middle record */
rec_t *page_get_middle_rec(page_t *page) /*!< in: page */
    MY_ATTRIBUTE((warn_unused_result));
#endif /* !UNIV_HOTBACKUP */
