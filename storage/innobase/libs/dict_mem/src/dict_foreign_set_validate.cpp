#include <innodb/dict_mem/dict_foreign_set_validate.h>

#include <innodb/dict_mem/dict_foreign_not_exists.h>

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
