#pragma once

#include <innodb/univ/univ.h>

struct mtr_t;

/** Write a log record of writing to the uncompressed header portion of a page.
 */
void page_zip_write_header_log(
    const byte *data, /*!< in: data on the uncompressed page */
    ulint length,     /*!< in: length of the data */
    mtr_t *mtr);      /*!< in: mini-transaction */
