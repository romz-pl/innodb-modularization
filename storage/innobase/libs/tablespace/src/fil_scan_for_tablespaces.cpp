#include <innodb/tablespace/fil_scan_for_tablespaces.h>

#include <innodb/tablespace/fil_system.h>

/** Discover tablespaces by reading the header from .ibd files.
@param[in]	directories	Directories to scan
@return DB_SUCCESS if all goes well */
dberr_t fil_scan_for_tablespaces(const std::string &directories) {
  return (fil_system->scan(directories));
}
