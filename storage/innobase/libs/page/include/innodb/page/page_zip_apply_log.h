#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

struct dict_index_t;

const byte *page_zip_apply_log(
    const byte *data, /*!< in: modification log */
    ulint size,       /*!< in: maximum length of the log, in bytes */
    rec_t **recs,     /*!< in: dense page directory,
                      sorted by address (indexed by
                      heap_no - PAGE_HEAP_NO_USER_LOW) */
    ulint n_dense,    /*!< in: size of recs[] */
    ulint trx_id_col, /*!< in: column number of trx_id in the index,
                   or ULINT_UNDEFINED if none */
    ulint heap_status,
    /*!< in: heap_no and status bits for
    the next record to uncompress */
    dict_index_t *index, /*!< in: index of the page */
    ulint *offsets);      /*!< in/out: work area for
                         rec_get_offsets_reverse() */
