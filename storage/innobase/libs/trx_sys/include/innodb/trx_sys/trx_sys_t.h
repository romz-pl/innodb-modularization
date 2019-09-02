#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_types/TrxSysMutex.h>
#include <innodb/trx_sys/trx_ut_list_t.h>
#include <innodb/trx_types/trx_ids_t.h>
#include <innodb/trx_types/Rsegs.h>
#include <innodb/trx_types/TrxIdSet.h>


class MVCC;

#ifndef UNIV_HOTBACKUP
/** The transaction system central memory data structure. */
struct trx_sys_t {
  TrxSysMutex mutex; /*!< mutex protecting most fields in
                     this structure except when noted
                     otherwise */

  MVCC *mvcc;                   /*!< Multi version concurrency control
                                manager */
  volatile trx_id_t max_trx_id; /*!< The smallest number not yet
                                assigned as a transaction id or
                                transaction number. This is declared
                                volatile because it can be accessed
                                without holding any mutex during
                                AC-NL-RO view creation. */
  std::atomic<trx_id_t> min_active_id;
  /*!< Minimal transaction id which is
  still in active state. */
  trx_ut_list_t serialisation_list;
  /*!< Ordered on trx_t::no of all the
  currenrtly active RW transactions */
#ifdef UNIV_DEBUG
  trx_id_t rw_max_trx_id; /*!< Max trx id of read-write
                          transactions which exist or existed */
#endif                    /* UNIV_DEBUG */

  char pad1[64];             /*!< To avoid false sharing */
  trx_ut_list_t rw_trx_list; /*!< List of active and committed in
                             memory read-write transactions, sorted
                             on trx id, biggest first. Recovered
                             transactions are always on this list. */

  char pad2[64];                /*!< To avoid false sharing */
  trx_ut_list_t mysql_trx_list; /*!< List of transactions created
                                for MySQL. All user transactions are
                                on mysql_trx_list. The rw_trx_list
                                can contain system transactions and
                                recovered transactions that will not
                                be in the mysql_trx_list.
                                mysql_trx_list may additionally contain
                                transactions that have not yet been
                                started in InnoDB. */

  trx_ids_t rw_trx_ids; /*!< Array of Read write transaction IDs
                        for MVCC snapshot. A ReadView would take
                        a snapshot of these transactions whose
                        changes are not visible to it. We should
                        remove transactions from the list before
                        committing in memory and releasing locks
                        to ensure right order of removal and
                        consistent snapshot. */

  char pad3[64]; /*!< To avoid false sharing */

  Rsegs rsegs; /*!< Vector of pointers to rollback
               segments. These rsegs are iterated
               and added to the end under a read
               lock. They are deleted under a write
               lock while the vector is adjusted.
               They are created and destroyed in
               single-threaded mode. */

  Rsegs tmp_rsegs; /*!< Vector of pointers to rollback
                   segments within the temp tablespace;
                   This vector is created and destroyed
                   in single-threaded mode so it is not
                   protected by any mutex because it is
                   read-only during multi-threaded
                   operation. */

  ulint rseg_history_len;
  /*!< Length of the TRX_RSEG_HISTORY
  list (update undo logs for committed
  transactions), protected by
  rseg->mutex */

  TrxIdSet rw_trx_set; /*!< Mapping from transaction id
                       to transaction instance */

  ulint n_prepared_trx; /*!< Number of transactions currently
                        in the XA PREPARED state */

  bool found_prepared_trx; /*!< True if XA PREPARED trxs are
                           found. */
};

#endif /* !UNIV_HOTBACKUP */

