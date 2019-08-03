#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/rec_t.h>

/** Sets a bit field within 2 bytes. */
void rec_set_bit_field_2(
    rec_t *rec,  /*!< in: pointer to record origin */
    ulint val,   /*!< in: value to set */
    ulint offs,  /*!< in: offset from the origin down */
    ulint mask,  /*!< in: mask used to filter bits */
    ulint shift); /*!< in: shift right applied after masking */

