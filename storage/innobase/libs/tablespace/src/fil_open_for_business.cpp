#include <innodb/tablespace/fil_open_for_business.h>

#include <innodb/tablespace/fil_system.h>

/** Free the Tablespace_files instance.
@param[in]	read_only_mode	true if InnoDB is started in read only mode.
@return DB_SUCCESS if all OK */
dberr_t fil_open_for_business(bool read_only_mode) {
  return (fil_system->prepare_open_for_business(read_only_mode));
}
