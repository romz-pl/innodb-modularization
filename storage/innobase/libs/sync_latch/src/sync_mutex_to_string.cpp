#include <innodb/sync_latch/sync_mutex_to_string.h>
#include <innodb/sync_latch/sync_latch_get_name.h>

#include <sstream>

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
