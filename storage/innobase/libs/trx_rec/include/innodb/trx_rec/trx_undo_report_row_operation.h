#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>
#include <innodb/univ/rec_t.h>

struct que_thr_t;
struct dict_index_t;
struct dtuple_t;
struct upd_t;

/** Writes information to an undo log about an insert, update, or a delete
 marking of a clustered index record. This information is used in a rollback of
 the transaction and in consistent reads that must look to the history of this
 transaction.
 @return DB_SUCCESS or error code */
dberr_t trx_undo_report_row_operation(
    ulint flags,                 /*!< in: if BTR_NO_UNDO_LOG_FLAG bit is
                                 set, does nothing */
    ulint op_type,               /*!< in: TRX_UNDO_INSERT_OP or
                                 TRX_UNDO_MODIFY_OP */
    que_thr_t *thr,              /*!< in: query thread */
    dict_index_t *index,         /*!< in: clustered index */
    const dtuple_t *clust_entry, /*!< in: in the case of an insert,
                                 index entry to insert into the
                                 clustered index, otherwise NULL */
    const upd_t *update,         /*!< in: in the case of an update,
                                 the update vector, otherwise NULL */
    ulint cmpl_info,             /*!< in: compiler info on secondary
                                 index updates */
    const rec_t *rec,            /*!< in: case of an update or delete
                                 marking, the record in the clustered
                                 index, otherwise NULL */
    const ulint *offsets,        /*!< in: rec_get_offsets(rec) */
    roll_ptr_t *roll_ptr)        /*!< out: rollback pointer to the
                                 inserted undo log record,
                                 0 if BTR_NO_UNDO_LOG
                                 flag was specified */
    MY_ATTRIBUTE((warn_unused_result));
