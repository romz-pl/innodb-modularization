#pragma once

#include <innodb/univ/univ.h>

/** Check if clone locator is valid
@param[in]	desc_loc	serialized descriptor
@param[in]	desc_len	descriptor length
@return true, if valid locator */
bool clone_validate_locator(const byte *desc_loc, uint desc_len);
