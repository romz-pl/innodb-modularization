#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_foreign_set.h>

/** Validate the search order in the foreign key set.
@param[in]	fk_set	the foreign key set to be validated
@return true if search order is fine in the set, false otherwise. */
bool dict_foreign_set_validate(const dict_foreign_set &fk_set);


struct dict_table_t;

/** Validate the search order in the foreign key sets of the table
(foreign_set and referenced_set).
@param[in]	table	table whose foreign key sets are to be validated
@return true if foreign key sets are fine, false otherwise. */
bool dict_foreign_set_validate(const dict_table_t &table);
