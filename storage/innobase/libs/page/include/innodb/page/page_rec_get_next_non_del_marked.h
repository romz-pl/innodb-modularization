#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

/** Gets the pointer to the next non delete-marked record on the page.
 If all subsequent records are delete-marked, then this function
 will return the supremum record.
 @return pointer to next non delete-marked record or pointer to supremum */
const rec_t *page_rec_get_next_non_del_marked(
    const rec_t *rec); /*!< in: pointer to record */
