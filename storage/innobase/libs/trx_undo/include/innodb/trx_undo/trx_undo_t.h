#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/page_no_t.h>
#include <innodb/disk/page_size_t.h>
#include <innodb/lst/lst.h>

#include "sql/handler.h"
#include "sql/xa.h"

struct trx_rseg_t;
struct buf_block_t;

/** Transaction undo log memory object; this is protected by the undo_mutex
in the corresponding transaction object */

struct trx_undo_t {
  /*-----------------------------*/
  ulint id;             /*!< undo log slot number within the
                        rollback segment */
  ulint type;           /*!< TRX_UNDO_INSERT or
                        TRX_UNDO_UPDATE */
  ulint state;          /*!< state of the corresponding undo log
                        segment */
  ibool del_marks;      /*!< relevant only in an update undo
                        log: this is TRUE if the transaction may
                        have delete marked records, because of
                        a delete of a row or an update of an
                        indexed field; purge is then
                        necessary; also TRUE if the transaction
                        has updated an externally stored
                        field */
  trx_id_t trx_id;      /*!< id of the trx assigned to the undo
                        log */
  XID xid;              /*!< X/Open XA transaction
                        identification */
  ibool dict_operation; /*!< TRUE if a dict operation trx */
  trx_rseg_t *rseg;     /*!< rseg where the undo log belongs */
  /*-----------------------------*/
  space_id_t space; /*!< space id where the undo log
                    placed */
  page_size_t page_size;
  page_no_t hdr_page_no;  /*!< page number of the header page in
                          the undo log */
  ulint hdr_offset;       /*!< header offset of the undo log on
                          the page */
  page_no_t last_page_no; /*!< page number of the last page in the
                          undo log; this may differ from
                          top_page_no during a rollback */
  ulint size;             /*!< current size in pages */
  /*-----------------------------*/
  ulint empty;              /*!< TRUE if the stack of undo log
                            records is currently empty */
  page_no_t top_page_no;    /*!< page number where the latest undo
                            log record was catenated; during
                            rollback the page from which the latest
                            undo record was chosen */
  ulint top_offset;         /*!< offset of the latest undo record,
                            i.e., the topmost element in the undo
                            log if we think of it as a stack */
  undo_no_t top_undo_no;    /*!< undo number of the latest record */
  buf_block_t *guess_block; /*!< guess for the buffer block where
                            the top page might reside */
  ulint withdraw_clock;     /*!< the withdraw clock value of the
                            buffer pool when guess_block was stored */
  /*-----------------------------*/
  UT_LIST_NODE_T(trx_undo_t) undo_list;
  /*!< undo log objects in the rollback
  segment are chained into lists */
};

