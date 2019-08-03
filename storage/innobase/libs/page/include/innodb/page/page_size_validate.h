#pragma once

#include <innodb/univ/univ.h>

/** Check that a page_size is correct for InnoDB.
If correct, set the associated page_size_shift which is the power of 2
for this page size.
@param[in]	page_size	Page Size to evaluate
@return an associated page_size_shift if valid, 0 if invalid. */
ulong page_size_validate(ulong page_size);
