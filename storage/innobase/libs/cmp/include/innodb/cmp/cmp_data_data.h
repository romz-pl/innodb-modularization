#pragma once

#include <innodb/univ/univ.h>

/*!< in: whether to check charsets */
/** Compare two data fields.
@param[in]	mtype	main type
@param[in]	prtype	precise type
@param[in]	is_asc	true=ascending, false=descending order
@param[in]	data1	data field
@param[in]	len1	length of data1 in bytes, or UNIV_SQL_NULL
@param[in]	data2	data field
@param[in]	len2	length of data2 in bytes, or UNIV_SQL_NULL
@return the comparison result of data1 and data2
@retval 0 if data1 is equal to data2
@retval negative if data1 is less than data2
@retval positive if data1 is greater than data2 */
int cmp_data_data(ulint mtype, ulint prtype, bool is_asc, const byte *data1,
                  ulint len1, const byte *data2, ulint len2)
    MY_ATTRIBUTE((warn_unused_result));
