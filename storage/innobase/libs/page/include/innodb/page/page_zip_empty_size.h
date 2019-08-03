#pragma once

#include <innodb/univ/univ.h>

/** Determine the guaranteed free space on an empty page.
 @return minimum payload size on the page */
ulint page_zip_empty_size(
    ulint n_fields, /*!< in: number of columns in the index */
    ulint zip_size) /*!< in: compressed page size in bytes */
    MY_ATTRIBUTE((const));
