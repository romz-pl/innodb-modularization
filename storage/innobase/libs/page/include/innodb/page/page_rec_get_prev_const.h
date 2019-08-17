#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

/** Gets the pointer to the previous record.
 @return pointer to previous record */
const rec_t *page_rec_get_prev_const(
    const rec_t *rec); /*!< in: pointer to record, must not be page
                       infimum */
