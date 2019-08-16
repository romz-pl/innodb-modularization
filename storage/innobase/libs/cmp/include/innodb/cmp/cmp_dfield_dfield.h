#pragma once

#include <innodb/univ/univ.h>

struct dfield_t;

/** Compare two data fields.
@param[in]	dfield1	data field; must have type field set
@param[in]	dfield2	data field
@param[in]	is_asc	true=ASC, false=DESC
@return the comparison result of dfield1 and dfield2
@retval 0 if dfield1 is equal to dfield2
@retval negative if dfield1 is less than dfield2
@retval positive if dfield1 is greater than dfield2 */
int cmp_dfield_dfield(const dfield_t *dfield1, const dfield_t *dfield2,
                      bool is_asc) MY_ATTRIBUTE((warn_unused_result));
