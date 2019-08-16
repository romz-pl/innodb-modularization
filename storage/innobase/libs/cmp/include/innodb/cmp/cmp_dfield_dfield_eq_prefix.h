#pragma once

#include <innodb/univ/univ.h>

struct dfield_t;

/** Compare two data fields.
@param[in] dfield1 data field
@param[in] dfield2 data field
@return the comparison result of dfield1 and dfield2
@retval true if dfield1 is equal to dfield2, or a prefix of dfield1
@retval false otherwise */
bool cmp_dfield_dfield_eq_prefix(const dfield_t *dfield1,
                                 const dfield_t *dfield2)
    MY_ATTRIBUTE((warn_unused_result));
