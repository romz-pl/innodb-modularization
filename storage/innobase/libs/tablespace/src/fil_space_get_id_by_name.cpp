#include <innodb/tablespace/fil_space_get_id_by_name.h>

#include <innodb/tablespace/fil_system.h>

/** Return the space ID based on the tablespace name.
The tablespace must be found in the tablespace memory cache.
@param[in]	name		Tablespace name
@return space ID if tablespace found, SPACE_UNKNOWN if space not. */
space_id_t fil_space_get_id_by_name(const char *name) {
  auto space = fil_system->get_space_by_name(name);

  return ((space == nullptr) ? SPACE_UNKNOWN : space->id);
}
