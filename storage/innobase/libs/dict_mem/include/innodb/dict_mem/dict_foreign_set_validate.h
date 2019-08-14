#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_foreign_set.h>

/** Validate the search order in the foreign key set.
@param[in]	fk_set	the foreign key set to be validated
@return true if search order is fine in the set, false otherwise. */
bool dict_foreign_set_validate(const dict_foreign_set &fk_set);
