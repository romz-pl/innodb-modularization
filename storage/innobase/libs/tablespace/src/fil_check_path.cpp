#include <innodb/tablespace/fil_check_path.h>

#include <innodb/tablespace/fil_system.h>

/** Check if a path is known to InnoDB.
@param[in]	path		Path to check
@return true if path is known to InnoDB */
bool fil_check_path(const std::string &path) {
  return (fil_system->check_path(path));
}
