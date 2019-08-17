#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

#include <zlib.h>

ibool page_zip_decompress_clust_ext(
    z_stream *d_stream,   /*!< in/out: compressed page stream */
    rec_t *rec,           /*!< in/out: record */
    const ulint *offsets, /*!< in: rec_get_offsets(rec) */
    ulint trx_id_col);     /*!< in: position of of DB_TRX_ID */
