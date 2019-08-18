#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

struct page_zip_des_t;

/** Parses a log record of compressing an index page.
 @return end of log record or NULL */
byte *page_zip_parse_compress(
    byte *ptr,                 /*!< in: buffer */
    byte *end_ptr,             /*!< in: buffer end */
    page_t *page,              /*!< out: uncompressed page */
    page_zip_des_t *page_zip); /*!< out: compressed page */
