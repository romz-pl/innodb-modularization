#include <innodb/tablespace/fil_space_acquire_silent.h>

#include <innodb/tablespace/fil_space_acquire_low.h>

/** Acquire a tablespace that may not exist.
Used by background threads that do not necessarily hold proper locks
for concurrency control.
@param[in]	space_id	Tablespace ID
@return the tablespace, or nullptr if missing or being deleted */
fil_space_t *fil_space_acquire_silent(space_id_t space_id) {
  return (fil_space_acquire_low(space_id, true));
}
