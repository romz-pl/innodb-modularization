#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

#include <zlib.h>

ibool page_zip_decompress_heap_no(
    z_stream *d_stream, /*!< in/out: compressed page stream */
    rec_t *rec,         /*!< in/out: record */
    ulint &heap_status); /*!< in/out: heap_no and status bits */
