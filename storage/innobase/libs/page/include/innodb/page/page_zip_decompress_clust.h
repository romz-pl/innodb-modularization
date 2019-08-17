#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>
#include <innodb/memory/mem_heap_t.h>

#include <zlib.h>

struct page_zip_des_t;
struct dict_index_t;

ibool page_zip_decompress_clust(
    page_zip_des_t *page_zip, /*!< in/out: compressed page */
    z_stream *d_stream,       /*!< in/out: compressed page stream */
    rec_t **recs,             /*!< in: dense page directory
                              sorted by address */
    ulint n_dense,            /*!< in: size of recs[] */
    dict_index_t *index,      /*!< in: the index of the page */
    ulint trx_id_col,         /*!< index of the trx_id column */
    ulint *offsets,           /*!< in/out: temporary offsets */
    mem_heap_t *heap);         /*!< in: temporary memory heap */
