#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

struct dtuple_t;

rec_t *rec_convert_dtuple_to_rec_old(
    byte *buf,              /*!< in: start address of the physical record */
    const dtuple_t *dtuple, /*!< in: data tuple */
    ulint n_ext);            /*!< in: number of externally stored columns */
