#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

const byte *page_zip_apply_log_ext(
    rec_t *rec,           /*!< in/out: record */
    const ulint *offsets, /*!< in: rec_get_offsets(rec) */
    ulint trx_id_col,     /*!< in: position of of DB_TRX_ID */
    const byte *data,     /*!< in: modification log */
    const byte *end);      /*!< in: end of modification log */
