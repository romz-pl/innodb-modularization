#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

rec_t *rec_copy_prefix_to_buf_old(
    const rec_t *rec, /*!< in: physical record */
    ulint n_fields,   /*!< in: number of fields to copy */
    ulint area_end,   /*!< in: end of the prefix data */
    byte **buf,       /*!< in/out: memory buffer for
                      the copied prefix, or NULL */
    size_t *buf_size); /*!< in/out: buffer size */
