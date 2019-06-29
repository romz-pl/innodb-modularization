#include <innodb/sync_latch/sync_file_created_get.h>
#include <innodb/sync_latch/create_tracker.h>

/** Get the string where the file was created. Its format is "name:line"
@param[in]	ptr		Latch instance
@return created information or "" if can't be found */
std::string sync_file_created_get(const void *ptr) {
  return (create_tracker->get(ptr));
}
