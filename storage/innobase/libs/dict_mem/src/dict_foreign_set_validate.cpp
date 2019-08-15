#include <innodb/dict_mem/dict_foreign_set_validate.h>

#include <innodb/dict_mem/dict_foreign_not_exists.h>
#include <innodb/dict_mem/dict_table_t.h>

#include <algorithm>
#include <iostream>

/** Validate the search order in the foreign key set.
@param[in]	fk_set	the foreign key set to be validated
@return true if search order is fine in the set, false otherwise. */
bool dict_foreign_set_validate(const dict_foreign_set &fk_set) {
  dict_foreign_not_exists not_exists(fk_set);

  dict_foreign_set::iterator it =
      std::find_if(fk_set.begin(), fk_set.end(), not_exists);

  if (it == fk_set.end()) {
    return (true);
  }

  dict_foreign_t *foreign = *it;
  std::cerr << "Foreign key lookup failed: " << *foreign;
  std::cerr << fk_set;
  ut_ad(0);
  return (false);
}


/** Validate the search order in the foreign key sets of the table
(foreign_set and referenced_set).
@param[in]	table	table whose foreign key sets are to be validated
@return true if foreign key sets are fine, false otherwise. */
bool dict_foreign_set_validate(const dict_table_t &table) {
  return (dict_foreign_set_validate(table.foreign_set) &&
          dict_foreign_set_validate(table.referenced_set));
}
