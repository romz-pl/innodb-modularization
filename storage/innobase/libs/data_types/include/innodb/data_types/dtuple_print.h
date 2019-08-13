#pragma once

#include <innodb/univ/univ.h>

#include <iosfwd>
#include <cstdio>

struct dtuple_t;

/** Print the contents of a tuple.
@param[out]	o	output stream
@param[in]	tuple	data tuple */
void dtuple_print(std::ostream &o, const dtuple_t *tuple);



/** The following function prints the contents of a tuple. */
void dtuple_print(FILE *f,               /*!< in: output stream */
                  const dtuple_t *tuple); /*!< in: tuple */


