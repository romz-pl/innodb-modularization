#pragma once

#include <innodb/univ/univ.h>

#include <zlib.h>

extern "C" {

void *page_zip_zalloc(void *opaque, /*!< in/out: memory heap */
                             uInt items, /*!< in: number of items to allocate */
                             uInt size);  /*!< in: size of an item in bytes */
}
