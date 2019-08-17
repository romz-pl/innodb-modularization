#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>
#include <innodb/page/FILE_LOGFILE.h>
#include <innodb/memory/mem_heap_t.h>

#include <zlib.h>

struct page_zip_des_t;
struct dict_index_t;

int page_zip_compress_clust(
    FILE_LOGFILE z_stream *c_stream, /*!< in/out: compressed page stream */
    const rec_t **recs,              /*!< in: dense page directory
                                     sorted by address */
    ulint n_dense,                   /*!< in: size of recs[] */
    dict_index_t *index,             /*!< in: the index of the page */
    ulint *n_blobs,                  /*!< in: 0; out: number of
                                     externally stored columns */
    ulint trx_id_col,                /*!< index of the trx_id column */
    byte *deleted,                   /*!< in: dense directory entry pointing
                                     to the head of the free list */
    byte *storage,                   /*!< in: end of dense page directory */
    mem_heap_t *heap);                /*!< in: temporary memory heap */
