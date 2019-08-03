#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

/** Returns the maximum combined size of records which can be inserted on top
of record heap if page is first reorganized.
@param[in]	page	index page
@param[in]	n_recs	number of records
@return maximum combined size for inserted records */
ulint page_get_max_insert_size_after_reorganize(const page_t *page,
                                                ulint n_recs);
