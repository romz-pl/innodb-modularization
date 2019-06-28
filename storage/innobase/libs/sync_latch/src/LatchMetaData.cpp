#include <innodb/sync_latch/LatchMetaData.h>

#ifndef UNIV_LIBRARY

/* Meta data for all the InnoDB latches. If the latch is not in recorded
here then it will be be considered for deadlock checks.  */
LatchMetaData latch_meta;


#ifndef UNIV_HOTBACKUP
/** Print the filename "basename" e.g., p = "/a/b/c/d/e.cc" -> p = "e.cc"
@param[in]	filename	Name from where to extract the basename
@return the basename */
const char *sync_basename(const char *filename) {
  const char *ptr = filename + strlen(filename) - 1;

  while (ptr > filename && *ptr != '/' && *ptr != '\\') {
    --ptr;
  }

  ++ptr;

  return (ptr);
}

/** String representation of the filename and line number where the
latch was created
@param[in]	id		Latch ID
@param[in]	created		Filename and line number where it was crated
@return the string representation */
std::string sync_mutex_to_string(latch_id_t id, const std::string &created) {
  std::ostringstream msg;

  msg << "Mutex " << sync_latch_get_name(id) << " "
      << "created " << created;

  return (msg.str());
}

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


#endif /* !UNIV_HOTBACKUP */

#endif /* UNIV_LIBRARY */
