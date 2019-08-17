#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

/** Returns the number of records before the given record in chain.
 The number includes infimum and supremum records.
 This is the inverse function of page_rec_get_nth().
 @return number of records */
ulint page_rec_get_n_recs_before(
    const rec_t *rec); /*!< in: the physical record */
