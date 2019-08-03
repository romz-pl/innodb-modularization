#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/rec_t.h>

#ifdef UNIV_DEBUG

/** Print the record offsets.
@param[in]    out         the output stream to which offsets are printed.
@param[in]    offsets     the field offsets of the record.
@return the output stream. */
std::ostream &rec_offs_print(std::ostream &out, const ulint *offsets);

#endif /* UNIV_DEBUG */
