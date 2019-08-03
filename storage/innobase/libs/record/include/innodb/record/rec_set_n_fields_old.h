#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/rec_t.h>

/** The following function is used to set the number of fields
 in an old-style record. */
void rec_set_n_fields_old(rec_t *rec,     /*!< in: physical record */
                          ulint n_fields); /*!< in: the number of fields */
