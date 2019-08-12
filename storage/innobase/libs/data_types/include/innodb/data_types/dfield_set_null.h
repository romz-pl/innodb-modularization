#pragma once

#include <innodb/univ/univ.h>

struct dfield_t;

/** Sets a data field to SQL NULL. */
void dfield_set_null(dfield_t *field); /*!< in/out: field */
