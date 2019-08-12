#pragma once

#include <innodb/univ/univ.h>

/** Get the length of the number of fields for any new style record.
@param[in]	n_fields	number of fields in the record
@return	length of specified number of fields */
uint8_t rec_get_n_fields_length(ulint n_fields);
