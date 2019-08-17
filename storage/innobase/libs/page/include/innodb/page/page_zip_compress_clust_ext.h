#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>
#include <innodb/page/FILE_LOGFILE.h>

#include <zlib.h>

struct page_zip_des_t;

int page_zip_compress_clust_ext(
    FILE_LOGFILE z_stream *c_stream, /*!< in/out: compressed page stream */
    const rec_t *rec,                /*!< in: record */
    const ulint *offsets,            /*!< in: rec_get_offsets(rec) */
    ulint trx_id_col,                /*!< in: position of of DB_TRX_ID */
    byte *deleted,                   /*!< in: dense directory entry pointing
                                     to the head of the free list */
    byte *storage,                   /*!< in: end of dense page directory */
    byte **externs,                  /*!< in/out: pointer to the next
                                     available BLOB pointer */
    ulint *n_blobs);                  /*!< in/out: number of
                                     externally stored columns */
