#include <innodb/tablespace/fil_space_set_imported.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/tablespace/fil_system.h>


/** Note that a tablespace has been imported.
It is initially marked as FIL_TYPE_IMPORT so that no logging is
done during the import process when the space ID is stamped to each page.
Now we change it to FIL_SPACE_TABLESPACE to start redo and undo logging.
NOTE: temporary tablespaces are never imported.
@param[in]	space_id	Tablespace ID */
void fil_space_set_imported(space_id_t space_id) {
  auto shard = fil_system->shard_by_id(space_id);

  shard->mutex_acquire();

  fil_space_t *space = shard->get_space_by_id(space_id);

  ut_ad(space->purpose == FIL_TYPE_IMPORT);
  space->purpose = FIL_TYPE_TABLESPACE;

  shard->mutex_release();
}
#endif /* !UNIV_HOTBACKUP */
