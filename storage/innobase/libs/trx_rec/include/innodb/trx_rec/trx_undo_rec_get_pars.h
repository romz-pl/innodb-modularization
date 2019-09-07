#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_types/trx_undo_rec_t.h>

struct type_cmpl_t;

/** Reads from an undo log record the general parameters.
 @return remaining part of undo log record after reading these values */
byte *trx_undo_rec_get_pars(
    trx_undo_rec_t *undo_rec, /*!< in: undo log record */
    ulint *type,              /*!< out: undo record type:
                              TRX_UNDO_INSERT_REC, ... */
    ulint *cmpl_info,         /*!< out: compiler info, relevant only
                              for update type records */
    bool *updated_extern,     /*!< out: true if we updated an
                              externally stored fild */
    undo_no_t *undo_no,       /*!< out: undo log record number */
    table_id_t *table_id,     /*!< out: table id */
    type_cmpl_t &type_cmpl);  /*!< out: type compilation info. */
