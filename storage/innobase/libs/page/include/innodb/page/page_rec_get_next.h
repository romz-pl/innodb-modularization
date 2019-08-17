#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

/** Gets the pointer to the next record on the page.
 @return pointer to next record */
rec_t *page_rec_get_next(rec_t *rec); /*!< in: pointer to record */
