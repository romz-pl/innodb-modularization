#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

/** Returns the sum of the sizes of the records in the record list
 excluding the infimum and supremum records.
 @return data in bytes */
ulint page_get_data_size(const page_t *page); /*!< in: index page */
