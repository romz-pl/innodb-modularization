#pragma once

#include <innodb/univ/univ.h>

struct dfield_t;

/** Pretty prints a dfield value according to its data type. Also the hex string
 is printed if a string contains non-printable characters. */
void dfield_print_also_hex(const dfield_t *dfield); /*!< in: dfield */
