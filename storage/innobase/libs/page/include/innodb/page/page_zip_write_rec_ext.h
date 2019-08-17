#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

struct page_zip_des_t;
struct dict_index_t;

byte *page_zip_write_rec_ext(
    page_zip_des_t *page_zip,  /*!< in/out: compressed page */
    const page_t *page,        /*!< in: page containing rec */
    const byte *rec,           /*!< in: record being written */
    const dict_index_t *index, /*!< in: record descriptor */
    const ulint *offsets,      /*!< in: rec_get_offsets(rec, index) */
    ulint create,              /*!< in: nonzero=insert, zero=update */
    ulint trx_id_col,          /*!< in: position of DB_TRX_ID */
    ulint heap_no,             /*!< in: heap number of rec */
    byte *storage,             /*!< in: end of dense page directory */
    byte *data);                /*!< in: end of modification log */
