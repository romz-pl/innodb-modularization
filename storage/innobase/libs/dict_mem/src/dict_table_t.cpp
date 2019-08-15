#include <innodb/dict_mem/dict_table_t.h>

#include <innodb/dict_mem/dict_table_mutex_alloc.h>
#include <innodb/dict_mem/dict_table_is_sdi.h>
#include <innodb/sync_mutex/mutex_enter.h>
#include <innodb/sync_mutex/mutex_exit.h>

/** Get reference count.
@return current value of n_ref_count */
uint64_t dict_table_t::get_ref_count() const {
    return (n_ref_count);
}

/** Acquire the table handle. */
void dict_table_t::acquire() {
  ut_ad(mutex_own(&dict_sys->mutex) || is_intrinsic());
  ++n_ref_count;
}

void dict_table_t::acquire_with_lock() {
  ut_ad(mutex_own(&dict_sys->mutex));
  ut_ad(!is_intrinsic());

  /* Acquiring the lock first, to prevent race between n_ref_count and
  stat_initialized in dict_table_close(). This lock makes sure the close
  code path either destroys the stats information before increasing
  n_ref_count, or finds the n_ref_count is not 0, so not to destry the stats
  information. */
  lock();
  ++n_ref_count;
  unlock();
}

/** Release the table handle. */
void dict_table_t::release() {
  ut_ad(n_ref_count > 0);
  --n_ref_count;
}

/** Lock the table handle. */
void dict_table_t::lock() {
#ifndef UNIV_HOTBACKUP
  os_once::do_or_wait_for_done(&mutex_created, dict_table_mutex_alloc, this);

  mutex_enter(mutex);
#endif /* !UNIV_HOTBACKUP */
}

/** Unlock the table handle. */
void dict_table_t::unlock() {
#ifndef UNIV_HOTBACKUP
  mutex_exit(mutex);
#endif /* !UNIV_HOTBACKUP */
}

/* GAP locks are skipped for DD tables and SDI tables
@return true if table is DD table or SDI table, else false */
bool dict_table_t::skip_gap_locks() const {
  return (is_dd_table || dict_table_is_sdi(id));
}

/** Determine if the table can support instant ADD COLUMN */
bool dict_table_t::support_instant_add() const {
  return (!DICT_TF_GET_ZIP_SSIZE(flags) && space != dict_sys_t_s_space_id &&
          !DICT_TF2_FLAG_IS_SET(this, DICT_TF2_FTS_HAS_DOC_ID) &&
          !is_temporary());
}
