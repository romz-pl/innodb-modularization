#include <innodb/cmp/cmp_dfield_dfield.h>

#include <innodb/cmp/cmp_data_data.h>
#include <innodb/data_types/dfield_check_typed.h>
#include <innodb/data_types/dfield_get_data.h>
#include <innodb/data_types/dfield_get_len.h>
#include <innodb/data_types/dfield_get_type.h>
#include <innodb/data_types/dfield_t.h>

/** Compare two data fields.
@param[in]	dfield1	data field; must have type field set
@param[in]	dfield2	data field
@param[in]	is_asc	true=ASC, false=DESC
@return the comparison result of dfield1 and dfield2
@retval 0 if dfield1 is equal to dfield2
@retval negative if dfield1 is less than dfield2
@retval positive if dfield1 is greater than dfield2 */
int cmp_dfield_dfield(const dfield_t *dfield1, const dfield_t *dfield2,
                      bool is_asc) {
  const dtype_t *type;

  ut_ad(dfield_check_typed(dfield1));

  type = dfield_get_type(dfield1);

  return (cmp_data_data(
      type->mtype, type->prtype, is_asc, (const byte *)dfield_get_data(dfield1),
      dfield_get_len(dfield1), (const byte *)dfield_get_data(dfield2),
      dfield_get_len(dfield2)));
}
