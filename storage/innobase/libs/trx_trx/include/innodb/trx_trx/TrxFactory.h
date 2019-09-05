#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_trx/trx_t.h>
#include <innodb/trx_trx/trx_init.h>
#include <innodb/trx_trx/flags.h>
#include <innodb/allocator/ut_zalloc_nokey.h>
#include <innodb/memory/macros.h>
#include <innodb/sync_mutex/mutex_create.h>
#include <innodb/sync_mutex/mutex_free.h>
#include <innodb/trx_types/flags.h>
#include <innodb/memory/mem_heap_create_typed.h>
#include <innodb/trx_trx/trx_named_savept_t.h>
#include <innodb/allocator/ut_free.h>


#include "sql/xa.h"

void lock_trx_lock_list_init(trx_lock_list_t *lock_list);
void lock_trx_alloc_locks(trx_t *trx);

/** For managing the life-cycle of the trx_t instance that we get
from the pool. */
struct TrxFactory {
  /** Initializes a transaction object. It must be explicitly started
  with trx_start_if_not_started() before using it. The default isolation
  level is TRX_ISO_REPEATABLE_READ.
  @param trx Transaction instance to initialise */
  static void init(trx_t *trx) {
    /* Explicitly call the constructor of the already
    allocated object. trx_t objects are allocated by
    ut_zalloc() in Pool::Pool() which would not call
    the constructors of the trx_t members. */
    new (&trx->mod_tables) trx_mod_tables_t();

    new (&trx->lock.rec_pool) lock_pool_t();

    new (&trx->lock.table_pool) lock_pool_t();

    new (&trx->lock.table_locks) lock_pool_t();

    new (&trx->hit_list) hit_list_t();

    trx_init(trx);

    trx->state = TRX_STATE_NOT_STARTED;

    trx->dict_operation_lock_mode = 0;

    trx->xid = UT_NEW_NOKEY(xid_t());

    trx->detailed_error =
        reinterpret_cast<char *>(ut_zalloc_nokey(MAX_DETAILED_ERROR_LEN));

    trx->lock.lock_heap = mem_heap_create_typed(1024, MEM_HEAP_FOR_LOCK_HEAP);

    lock_trx_lock_list_init(&trx->lock.trx_locks);

    UT_LIST_INIT(trx->trx_savepoints, &trx_named_savept_t::trx_savepoints);

    mutex_create(LATCH_ID_TRX, &trx->mutex);
    mutex_create(LATCH_ID_TRX_UNDO, &trx->undo_mutex);

    lock_trx_alloc_locks(trx);
  }

  /** Release resources held by the transaction object.
  @param trx the transaction for which to release resources */
  static void destroy(trx_t *trx) {
    ut_a(trx->magic_n == TRX_MAGIC_N);
    ut_ad(!trx->in_rw_trx_list);
    ut_ad(!trx->in_mysql_trx_list);

    ut_a(trx->lock.wait_lock == NULL);
    ut_a(trx->lock.wait_thr == NULL);

    ut_a(!trx->has_search_latch);

    ut_a(trx->dict_operation_lock_mode == 0);

    if (trx->lock.lock_heap != NULL) {
      mem_heap_free(trx->lock.lock_heap);
      trx->lock.lock_heap = NULL;
    }

    ut_a(UT_LIST_GET_LEN(trx->lock.trx_locks) == 0);

    UT_DELETE(trx->xid);
    ut_free(trx->detailed_error);

    mutex_free(&trx->mutex);
    mutex_free(&trx->undo_mutex);

    trx->mod_tables.~trx_mod_tables_t();

    ut_ad(trx->read_view == NULL);

    if (!trx->lock.rec_pool.empty()) {
      /* See lock_trx_alloc_locks() why we only free
      the first element. */

      ut_free(trx->lock.rec_pool[0]);
    }

    if (!trx->lock.table_pool.empty()) {
      /* See lock_trx_alloc_locks() why we only free
      the first element. */

      ut_free(trx->lock.table_pool[0]);
    }

    trx->lock.rec_pool.~lock_pool_t();

    trx->lock.table_pool.~lock_pool_t();

    trx->lock.table_locks.~lock_pool_t();

    trx->hit_list.~hit_list_t();
  }

  /** Enforce any invariants here, this is called before the transaction
  is added to the pool.
  @return true if all OK */
  static bool debug(const trx_t *trx) {
    ut_a(trx->error_state == DB_SUCCESS);

    ut_a(trx->magic_n == TRX_MAGIC_N);

    ut_ad(!trx->read_only);

    ut_ad(trx->state == TRX_STATE_NOT_STARTED ||
          trx->state == TRX_STATE_FORCED_ROLLBACK);

    ut_ad(trx->dict_operation == TRX_DICT_OP_NONE);

    ut_ad(trx->mysql_thd == 0);

    ut_ad(!trx->in_rw_trx_list);
    ut_ad(!trx->in_mysql_trx_list);

    ut_a(trx->lock.wait_thr == NULL);
    ut_a(trx->lock.wait_lock == NULL);

    ut_a(!trx->has_search_latch);

    ut_a(trx->dict_operation_lock_mode == 0);

    ut_a(UT_LIST_GET_LEN(trx->lock.trx_locks) == 0);

    ut_ad(trx->autoinc_locks == NULL);

    ut_ad(trx->lock.table_locks.empty());

    ut_ad(!trx->abort);

    ut_ad(trx->hit_list.empty());

    ut_ad(trx->killed_by == 0);

    return (true);
  }
};
