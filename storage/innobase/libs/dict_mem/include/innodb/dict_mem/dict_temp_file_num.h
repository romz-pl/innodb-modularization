#pragma once

#include <innodb/univ/univ.h>

/** An interger randomly initialized at startup used to make a temporary
table name as unuique as possible. */
extern ib_uint32_t dict_temp_file_num;
