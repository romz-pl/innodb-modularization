#pragma once

#include <innodb/univ/univ.h>

/** Returns nonzero if the default bit is set in nth field of rec.
@return nonzero if default bit is set */
ulint rec_offs_nth_default(const ulint *offsets, ulint n);
