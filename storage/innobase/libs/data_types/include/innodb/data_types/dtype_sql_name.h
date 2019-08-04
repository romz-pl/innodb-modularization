#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

/** Returns the type's SQL name (e.g. BIGINT UNSIGNED) from mtype,prtype,len
@param[in]	mtype	main type
@param[in]	prtype	precise type
@param[in]	len	length
@param[out]	name	SQL name
@param[in]	name_sz	size of the name buffer
@return the SQL type name */
char *dtype_sql_name(unsigned mtype, unsigned prtype, unsigned len, char *name,
                     unsigned name_sz);
#endif /* !UNIV_HOTBACKUP */
