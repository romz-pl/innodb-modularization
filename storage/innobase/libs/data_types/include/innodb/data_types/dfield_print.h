#pragma once

#include <innodb/univ/univ.h>

#include <iosfwd>

struct dfield_t;

/** Print the contents of a tuple.
@param[out]	o	output stream
@param[in]	field	array of data fields
@param[in]	n	number of data fields */
void dfield_print(std::ostream &o, const dfield_t *field, ulint n);
