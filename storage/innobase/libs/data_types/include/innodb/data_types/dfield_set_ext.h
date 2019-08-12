#pragma once

#include <innodb/univ/univ.h>

struct dfield_t;

/** Sets the "external storage" flag */
void dfield_set_ext(dfield_t *field); /*!< in/out: field */
