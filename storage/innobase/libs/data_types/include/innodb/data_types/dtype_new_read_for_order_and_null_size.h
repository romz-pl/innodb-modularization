#pragma once

#include <innodb/univ/univ.h>


#ifndef UNIV_HOTBACKUP

struct dtype_t;

void dtype_new_read_for_order_and_null_size(
    dtype_t *type,   /*!< in: type struct */
    const byte *buf); /*!< in: buffer for stored type order info */

#endif
