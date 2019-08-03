#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/rec_t.h>

/** Validates the consistency of a physical record.
 @return true if ok */
ibool rec_validate(
    const rec_t *rec,      /*!< in: physical record */
    const ulint *offsets); /*!< in: array returned by rec_get_offsets() */
