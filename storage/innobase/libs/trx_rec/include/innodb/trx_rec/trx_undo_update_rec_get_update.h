#pragma once

#include <innodb/univ/univ.h>

#include <innodb/memory/mem_heap_t.h>

struct dict_index_t;
struct type_cmpl_t;
struct trx_t;
struct upd_t;
namespace lob{ struct undo_vers_t; }

/** Builds an update vector based on a remaining part of an undo log record.
 @return remaining part of the record, NULL if an error detected, which
 means that the record is corrupted. */
byte *trx_undo_update_rec_get_update(
    const byte *ptr,            /*!< in: remaining part in update undo log
                                record, after reading the row reference
                                NOTE that this copy of the undo log record must
                                be preserved as long as the update vector is
                                used, as we do NOT copy the data in the
                                record! */
    const dict_index_t *index,  /*!< in: clustered index */
    ulint type,                 /*!< in: TRX_UNDO_UPD_EXIST_REC,
                                TRX_UNDO_UPD_DEL_REC, or
                                TRX_UNDO_DEL_MARK_REC; in the last case,
                                only trx id and roll ptr fields are added to
                                the update vector */
    trx_id_t trx_id,            /*!< in: transaction id from this undo record */
    roll_ptr_t roll_ptr,        /*!< in: roll pointer from this undo record */
    ulint info_bits,            /*!< in: info bits from this undo record */
    trx_t *trx,                 /*!< in: transaction */
    mem_heap_t *heap,           /*!< in: memory heap from which the memory
                                needed is allocated */
    upd_t **upd,                /*!< out, own: update vector */
    lob::undo_vers_t *lob_undo, /*!< out: LOB undo information. */
    type_cmpl_t &type_cmpl);    /*!< out: type compilation info */
