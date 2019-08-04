#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

struct dtype_t;

/** Reads to a type the stored information which determines its alphabetical
ordering and the storage size of an SQL NULL value.
@param[in]	type	type struct
@param[in]	buf	buffer for the stored order info */
void dtype_read_for_order_and_null_size(dtype_t *type, const byte *buf);



/** Stores for a type the information which determines its alphabetical ordering
 and the storage size of an SQL NULL value. This is the >= 4.1.x storage
 format. */
void dtype_new_store_for_order_and_null_size(
    byte *buf,           /*!< in: buffer for
                         DATA_NEW_ORDER_NULL_TYPE_BUF_SIZE
                         bytes where we store the info */
    const dtype_t *type, /*!< in: type struct */
    ulint prefix_len);    /*!< in: prefix length to
                      replace type->len, or 0 */


#endif
