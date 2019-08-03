#pragma once

#include <innodb/univ/univ.h>

#include <innodb/page/page_t.h>

/** Returns the offset stored in the given header field.
 @return offset from the start of the page, or 0 */
ulint page_header_get_offs(const page_t *page, /*!< in: page */
                           ulint field)        /*!< in: PAGE_FREE, ... */
    MY_ATTRIBUTE((warn_unused_result));
