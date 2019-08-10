#pragma once

#include <innodb/univ/univ.h>

/** Choose lowest descriptor version between reference locator
and currently supported version.
@param[in]	ref_loc	reference locator
@return chosen version */
uint choose_desc_version(const byte *ref_loc);
