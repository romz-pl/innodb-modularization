#include <innodb/tablespace/fil_close_tablespace.h>

#include <innodb/allocator/ut_free.h>
#include <innodb/io/Fil_path.h>
#include <innodb/io/os_file_delete_if_exists.h>
#include <innodb/io/pfs.h>
#include <innodb/sync_rw/rw_lock_x_lock.h>
#include <innodb/sync_rw/rw_lock_x_unlock.h>
#include <innodb/tablespace/buf_remove_t.h>
#include <innodb/tablespace/fil_space_free.h>
#include <innodb/tablespace/fil_space_t.h>
#include <innodb/tablespace/fil_system.h>
#include <innodb/tablespace/fsp_is_system_or_temp_tablespace.h>
#include <innodb/tablespace/fsp_is_undo_tablespace.h>

void buf_LRU_flush_or_remove_pages(space_id_t id, buf_remove_t buf_remove, const trx_t *trx, bool strict = true);

/** Closes a single-table tablespace. The tablespace must be cached in the
memory cache. Free all pages used by the tablespace.
@param[in,out]	trx		Transaction covering the close
@param[in]	space_id	Tablespace ID
@return DB_SUCCESS or error */
dberr_t fil_close_tablespace(trx_t *trx, space_id_t space_id) {
  char *path = nullptr;
  fil_space_t *space = nullptr;

  ut_ad(!fsp_is_undo_tablespace(space_id));
  ut_ad(!fsp_is_system_or_temp_tablespace(space_id));

  auto shard = fil_system->shard_by_id(space_id);

  dberr_t err;

  err = shard->space_check_pending_operations(space_id, space, &path);

  if (err != DB_SUCCESS) {
    return (err);
  }

  ut_a(path != nullptr);

  rw_lock_x_lock(&space->latch);

#ifndef UNIV_HOTBACKUP
  /* Invalidate in the buffer pool all pages belonging to the
  tablespace. Since we have set space->stop_new_ops = true, readahead
  or ibuf merge can no longer read more pages of this tablespace to the
  buffer pool. Thus we can clean the tablespace out of the buffer pool
  completely and permanently. The flag stop_new_ops also prevents
  fil_flush() from being applied to this tablespace. */

  buf_LRU_flush_or_remove_pages(space_id, BUF_REMOVE_FLUSH_WRITE, trx);
#endif /* !UNIV_HOTBACKUP */

  /* If the free is successful, the X lock will be released before
  the space memory data structure is freed. */

  if (!fil_space_free(space_id, true)) {
    rw_lock_x_unlock(&space->latch);
    err = DB_TABLESPACE_NOT_FOUND;
  } else {
    err = DB_SUCCESS;
  }

  /* If it is a delete then also delete any generated files, otherwise
  when we drop the database the remove directory will fail. */

  char *cfg_name = Fil_path::make_cfg(path);

  if (cfg_name != nullptr) {
    os_file_delete_if_exists(innodb_data_file_key, cfg_name, nullptr);

    ut_free(cfg_name);
  }

  char *cfp_name = Fil_path::make_cfp(path);

  if (cfp_name != nullptr) {
    os_file_delete_if_exists(innodb_data_file_key, cfp_name, nullptr);

    ut_free(cfp_name);
  }

  ut_free(path);

  return (err);
}
