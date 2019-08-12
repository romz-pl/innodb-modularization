#pragma once

#include <innodb/univ/univ.h>

struct dfield_t;

#ifdef UNIV_DEBUG

void *dfield_get_data(const dfield_t *field) /*!< in: field */
    MY_ATTRIBUTE((warn_unused_result));

#else /* UNIV_DEBUG */

#define dfield_get_data(field) ((field)->data)

#endif /* UNIV_DEBUG */
