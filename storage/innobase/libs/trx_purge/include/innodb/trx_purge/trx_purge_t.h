#pragma once

#include <innodb/univ/univ.h>

#include <innodb/read/ReadView.h>
#include <innodb/sync_rw/rw_lock_t.h>
#include <innodb/trx_purge/purge_iter_t.h>
#include <innodb/trx_types/trx_rseg_t.h>
#include <innodb/trx_types/purge_pq_t.h>
#include <innodb/trx_types/PQMutex.h>
#include <innodb/trx_purge/Truncate.h>
#include <innodb/memory/mem_heap_t.h>
#include <innodb/trx_purge/purge_state_t.h>

struct TrxUndoRsegsIterator;
struct sess_t;
struct trx_t;

typedef struct que_fork_t que_t;

/** The control structure used in the purge operation */
struct trx_purge_t {
  sess_t *sess; /*!< System session running the purge
                query */
  trx_t *trx;   /*!< System transaction running the
                purge query: this trx is not in the
                trx list of the trx system and it
                never ends */
#ifndef UNIV_HOTBACKUP
  rw_lock_t latch;              /*!< The latch protecting the purge
                                view. A purge operation must acquire an
                                x-latch here for the instant at which
                                it changes the purge view: an undo
                                log operation can prevent this by
                                obtaining an s-latch here. It also
                                protects state and running */
#endif                          /* !UNIV_HOTBACKUP */
  os_event_t event;             /*!< State signal event */
  ulint n_stop;                 /*!< Counter to track number stops */
  volatile bool running;        /*!< true, if purge is active,
                                we check this without the latch too */
  volatile purge_state_t state; /*!< Purge coordinator thread states,
                                we check this in several places
                                without holding the latch. */
  que_t *query;                 /*!< The query graph which will do the
                                parallelized purge operation */
  ReadView view;                /*!< The purge will not remove undo logs
                                which are >= this view (purge view) */
  bool view_active;             /*!< true if view is active */
  volatile ulint n_submitted;   /*!< Count of total tasks submitted
                                to the task queue */
  volatile ulint n_completed;   /*!< Count of total tasks completed */

  /*------------------------------*/
  /* The following two fields form the 'purge pointer' which advances
  during a purge, and which is used in history list truncation */

  purge_iter_t iter;  /* Limit up to which we have read and
                      parsed the UNDO log records.  Not
                      necessarily purged from the indexes.
                      Note that this can never be less than
                      the limit below, we check for this
                      invariant in trx0purge.cc */
  purge_iter_t limit; /* The 'purge pointer' which advances
                      during a purge, and which is used in
                      history list truncation */
#ifdef UNIV_DEBUG
  purge_iter_t done; /* Indicate 'purge pointer' which have
                     purged already accurately. */
#endif               /* UNIV_DEBUG */
  /*-----------------------------*/
  ibool next_stored;     /*!< TRUE if the info of the next record
                         to purge is stored below: if yes, then
                         the transaction number and the undo
                         number of the record are stored in
                         purge_trx_no and purge_undo_no above */
  trx_rseg_t *rseg;      /*!< Rollback segment for the next undo
                         record to purge */
  page_no_t page_no;     /*!< Page number for the next undo
                         record to purge, page number of the
                         log header, if dummy record */
  ulint offset;          /*!< Page offset for the next undo
                         record to purge, 0 if the dummy
                         record */
  page_no_t hdr_page_no; /*!< Header page of the undo log where
                         the next record to purge belongs */
  ulint hdr_offset;      /*!< Header byte offset on the page */

  TrxUndoRsegsIterator *rseg_iter; /*!< Iterator to get the next rseg
                                   to process */

  purge_pq_t *purge_queue; /*!< Binary min-heap, ordered on
                           TrxUndoRsegs::trx_no. It is protected
                           by the pq_mutex */
  PQMutex pq_mutex;        /*!< Mutex protecting purge_queue */

  undo::Truncate undo_trunc; /*!< Track UNDO tablespace marked
                             for truncate. */

  mem_heap_t *heap; /*!< Heap for reading the undo log
                    records */
};
