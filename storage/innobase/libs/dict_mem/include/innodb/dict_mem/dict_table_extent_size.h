#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/page_no_t.h>

struct dict_table_t;

/** Determine the extent size (in pages) for the given table
@param[in]	table	the table whose extent size is being
                        calculated.
@return extent size in pages (256, 128 or 64) */
page_no_t dict_table_extent_size(const dict_table_t *table);
