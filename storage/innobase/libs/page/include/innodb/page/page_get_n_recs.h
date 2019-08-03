#pragma once

#include <innodb/univ/univ.h>

#include <innodb/page/page_t.h>

/** Gets the number of user records on page (the infimum and supremum records
 are not user records).
 @return number of user records */
ulint page_get_n_recs(const page_t *page); /*!< in: index page */
