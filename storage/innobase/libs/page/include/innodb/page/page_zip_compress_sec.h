#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>
#include <innodb/page/FILE_LOGFILE.h>

#include <zlib.h>

struct page_zip_des_t;

int page_zip_compress_sec(
    FILE_LOGFILE z_stream *c_stream, /*!< in/out: compressed page stream */
    const rec_t **recs,              /*!< in: dense page directory
                                     sorted by address */
    ulint n_dense);                   /*!< in: size of recs[] */
