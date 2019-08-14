#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

/** TRUE if the record is on a page in compact format.
 @return nonzero if in compact format */
ulint page_rec_is_comp(const rec_t *rec); /*!< in: record */
