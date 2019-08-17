#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

/** The following is used to validate a record on a page. This function
 differs from rec_validate as it can also check the n_owned field and
 the heap_no field.
 @return true if ok */
ibool page_rec_validate(
    const rec_t *rec,      /*!< in: physical record */
    const ulint *offsets); /*!< in: array returned by rec_get_offsets() */
