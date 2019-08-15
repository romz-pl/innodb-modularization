#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_mem/dict_index_get_lock.h>
#include <innodb/dict_mem/dict_index_get_online_status.h>
#include <innodb/sync_rw/rw_lock_own.h>

/** Sets the status of online index creation. */
UNIV_INLINE
void dict_index_set_online_status(
    dict_index_t *index,             /*!< in/out: index */
    enum online_index_status status) /*!< in: status */
{
  ut_ad(!(index->type & DICT_FTS));
  ut_ad(rw_lock_own(dict_index_get_lock(index), RW_LOCK_X));

#ifdef UNIV_DEBUG
  switch (dict_index_get_online_status(index)) {
    case ONLINE_INDEX_COMPLETE:
    case ONLINE_INDEX_CREATION:
      break;
    case ONLINE_INDEX_ABORTED:
      ut_ad(status == ONLINE_INDEX_ABORTED_DROPPED);
      break;
    case ONLINE_INDEX_ABORTED_DROPPED:
      ut_error;
  }
#endif /* UNIV_DEBUG */

  index->online_status = status;
  ut_ad(dict_index_get_online_status(index) == status);
}
