#include <innodb/tablespace/fil_space_set_flags.h>

#include <innodb/sync_rw/rw_lock_x_lock.h>
#include <innodb/sync_rw/rw_lock_x_unlock.h>
#include <innodb/tablespace/fsp_flags_is_valid.h>
#include <innodb/tablespace/fil_space_t.h>

#include <limits>

/** Sets the flags of the tablespace. The tablespace must be locked
in MDL_EXCLUSIVE MODE.
@param[in]	space	tablespace in-memory struct
@param[in]	flags	tablespace flags */
void fil_space_set_flags(fil_space_t *space, uint32_t flags) {
  ut_ad(fsp_flags_is_valid(flags));

  rw_lock_x_lock(&space->latch);

  ut_a(flags < std::numeric_limits<uint32_t>::max());
  space->flags = (uint32_t)flags;

  rw_lock_x_unlock(&space->latch);
}
