#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_mem/dict_index_get_online_status.h>

/** Determines if a secondary index is being or has been created online,
 or if the table is being rebuilt online, allowing concurrent modifications
 to the table.
 @retval true if the index is being or has been built online, or
 if this is a clustered index and the table is being or has been rebuilt online
 @retval false if the index has been created or the table has been
 rebuilt completely */
UNIV_INLINE
bool dict_index_is_online_ddl(const dict_index_t *index) /*!< in: index */
{
#ifdef UNIV_DEBUG
  if (index->is_clustered()) {
    switch (dict_index_get_online_status(index)) {
      case ONLINE_INDEX_CREATION:
        return (true);
      case ONLINE_INDEX_COMPLETE:
        return (false);
      case ONLINE_INDEX_ABORTED:
      case ONLINE_INDEX_ABORTED_DROPPED:
        break;
    }
    ut_ad(0);
    return (false);
  }
#endif /* UNIV_DEBUG */

  return (UNIV_UNLIKELY(dict_index_get_online_status(index) !=
                        ONLINE_INDEX_COMPLETE));
}
