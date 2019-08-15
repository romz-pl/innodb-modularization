#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_mem/online_index_status.h>


/** Gets the status of online index creation.
 @return the status */
UNIV_INLINE
enum online_index_status dict_index_get_online_status(
    const dict_index_t *index) /*!< in: secondary index */
{
  enum online_index_status status;

  status = (enum online_index_status)index->online_status;

  /* Without the index->lock protection, the online
  status can change from ONLINE_INDEX_CREATION to
  ONLINE_INDEX_COMPLETE (or ONLINE_INDEX_ABORTED) in
  row_log_apply() once log application is done. So to make
  sure the status is ONLINE_INDEX_CREATION or ONLINE_INDEX_COMPLETE
  you should always do the recheck after acquiring index->lock */

#ifdef UNIV_DEBUG
  switch (status) {
    case ONLINE_INDEX_COMPLETE:
    case ONLINE_INDEX_CREATION:
    case ONLINE_INDEX_ABORTED:
    case ONLINE_INDEX_ABORTED_DROPPED:
      return (status);
  }
  ut_error;
#endif /* UNIV_DEBUG */
  return (status);
}
