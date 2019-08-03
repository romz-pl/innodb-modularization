#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/rec_t.h>

/** The following function tells if a new-style record is instant record or not
@param[in]	rec	new-style record
@return true if it's instant affected */
bool rec_get_instant_flag_new(const rec_t *rec);
