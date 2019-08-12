#pragma once

#include <innodb/univ/univ.h>

struct dfield_t;

#ifdef UNIV_DEBUG

/** Gets pointer to the type struct of SQL data field.
 @return pointer to the type struct */
dtype_t *dfield_get_type(const dfield_t *field) /*!< in: SQL data field */
    MY_ATTRIBUTE((warn_unused_result));

#else /* UNIV_DEBUG */

#define dfield_get_type(field) (&(field)->type)

#endif /* UNIV_DEBUG */
