#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>
#include <innodb/univ/rec_t.h>
#include <innodb/memory/mem_heap_t.h>

#include <zlib.h>

struct page_zip_des_t;
struct dict_index_t;

#include <innodb/page/FILE_LOGFILE.h>

int page_zip_compress_node_ptrs(
    FILE_LOGFILE z_stream *c_stream, /*!< in/out: compressed page stream */
    const rec_t **recs,              /*!< in: dense page directory
                                     sorted by address */
    ulint n_dense,                   /*!< in: size of recs[] */
    dict_index_t *index,             /*!< in: the index of the page */
    byte *storage,                   /*!< in: end of dense page directory */
    mem_heap_t *heap);                /*!< in: temporary memory heap */
