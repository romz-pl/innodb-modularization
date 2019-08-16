#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

struct dict_index_t;


/** Copies the first n fields of a physical record to a new physical record in
 a buffer.
 @return own: copied record */
rec_t *rec_copy_prefix_to_buf(
    const rec_t *rec,          /*!< in: physical record */
    const dict_index_t *index, /*!< in: record descriptor */
    ulint n_fields,            /*!< in: number of fields
                               to copy */
    byte **buf,                /*!< in/out: memory buffer
                               for the copied prefix,
                               or NULL */
    size_t *buf_size           /*!< in/out: buffer size */
);
