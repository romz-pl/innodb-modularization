#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/rec_t.h>

/** Used to check the consistency of a record on a page.
 @return true if succeed */
ibool page_rec_check(const rec_t *rec); /*!< in: record */
