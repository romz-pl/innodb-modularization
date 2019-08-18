#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_ZIP_DEBUG

void page_zip_hexdump_func(
    const char *name, /*!< in: name of the data structure */
    const void *buf,  /*!< in: data */
    ulint size);       /*!< in: length of the data, in bytes */

#endif

