#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

struct dict_index_t;
struct dtuple_t;

rec_t *rec_convert_dtuple_to_rec_new(
    byte *buf,                 /*!< in: start address of
                               the physical record */
    const dict_index_t *index, /*!< in: record descriptor */
    const dtuple_t *dtuple);    /*!< in: data tuple */
