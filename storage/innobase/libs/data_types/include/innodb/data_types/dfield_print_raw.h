#pragma once

#include <innodb/univ/univ.h>

#include <cstdio>

struct dfield_t;

void dfield_print_raw(FILE *f,                /*!< in: output stream */
                             const dfield_t *dfield);
