#include <innodb/tablespace/fil_tablespace_lookup_for_recovery.h>

#include <innodb/tablespace/fil_system.h>

/** Lookup the tablespace ID.
@param[in]	space_id		Tablespace ID to lookup
@return true if the space ID is known. */
bool fil_tablespace_lookup_for_recovery(space_id_t space_id) {
  return (fil_system->lookup_for_recovery(space_id));
}
