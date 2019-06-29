#include <innodb/sync_latch/sync_latch_get_name.h>

#ifndef UNIV_LIBRARY
#ifdef UNIV_PFS_MUTEX
#ifndef UNIV_HOTBACKUP

/** Get the latch name from a sync level
@param[in]	level		Latch level to lookup
@return NULL if not found. */
const char *sync_latch_get_name(latch_level_t level) {
  LatchMetaData::const_iterator end = latch_meta.end();

  /* Linear scan should be OK, this should be extremely rare. */

  for (LatchMetaData::const_iterator it = latch_meta.begin(); it != end; ++it) {
    if (*it == NULL || (*it)->get_id() == LATCH_ID_NONE) {
      continue;

    } else if ((*it)->get_level() == level) {
      return ((*it)->get_name());
    }
  }

  return (0);
}



#endif
#endif
#endif
