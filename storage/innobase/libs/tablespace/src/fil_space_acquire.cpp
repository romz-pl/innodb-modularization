#include <innodb/tablespace/fil_space_acquire.h>

#include <innodb/tablespace/fil_space_acquire_low.h>

/** Acquire a tablespace when it could be dropped concurrently.
Used by background threads that do not necessarily hold proper locks
for concurrency control.
@param[in]	space_id	Tablespace ID
@return the tablespace, or nullptr if missing or being deleted */
fil_space_t *fil_space_acquire(space_id_t space_id) {
  return (fil_space_acquire_low(space_id, false));
}
