#pragma once

#include <innodb/univ/univ.h>

/** Reads from an undo log update record the system field values of the old
 version.
 @return remaining part of undo log record after reading these values */
byte *trx_undo_update_rec_get_sys_cols(
    const byte *ptr,      /*!< in: remaining part of undo
                          log record after reading
                          general parameters */
    trx_id_t *trx_id,     /*!< out: trx id */
    roll_ptr_t *roll_ptr, /*!< out: roll ptr */
    ulint *info_bits);    /*!< out: info bits state */
