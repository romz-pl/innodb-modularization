#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

struct dict_index_t;

/** Write the index information for the compressed page.
 @return used size of buf */
ulint page_zip_fields_encode(
    ulint n,                   /*!< in: number of fields
                               to compress */
    const dict_index_t *index, /*!< in: index comprising
                               at least n fields */
    ulint trx_id_pos,
    /*!< in: position of the trx_id column
    in the index, or ULINT_UNDEFINED if
    this is a non-leaf page */
    byte *buf); /*!< out: buffer of (n + 1) * 2 bytes */
