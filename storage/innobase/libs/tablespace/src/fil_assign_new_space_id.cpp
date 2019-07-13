#include <innodb/tablespace/fil_assign_new_space_id.h>

#include <innodb/tablespace/fil_system.h>

/** Assigns a new space id for a new single-table tablespace. This works
simply by incrementing the global counter. If 4 billion id's is not enough,
we may need to recycle id's.
@param[out]	space_id		Set this to the new tablespace ID
@return true if assigned, false if not */
bool fil_assign_new_space_id(space_id_t *space_id) {
  return (fil_system->assign_new_space_id(space_id));
}
