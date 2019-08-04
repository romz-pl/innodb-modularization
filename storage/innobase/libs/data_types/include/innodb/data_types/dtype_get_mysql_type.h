#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

struct dtype_t;

/** Gets the MySQL type code from a dtype.
 @return MySQL type code; this is NOT an InnoDB type code! */
ulint dtype_get_mysql_type(const dtype_t *type); /*!< in: type struct */

#endif
