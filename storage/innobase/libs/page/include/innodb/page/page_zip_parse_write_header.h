#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

struct page_zip_des_t;

/** Parses a log record of writing to the header of a page.
 @return end of log record or NULL */
byte *page_zip_parse_write_header(
    byte *ptr,                 /*!< in: redo log buffer */
    byte *end_ptr,             /*!< in: redo log buffer end */
    page_t *page,              /*!< in/out: uncompressed page */
    page_zip_des_t *page_zip); /*!< in/out: compressed page */
