#include <innodb/tablespace/fil_space_create.h>

#include <innodb/tablespace/fil_system.h>
#include <innodb/trx_sys/flags.h>

bool clone_mark_abort(bool force);
void clone_mark_active();

/** Create a space memory object and put it to the fil_system hash table.
The tablespace name is independent from the tablespace file-name.
Error messages are issued to the server log.
@param[in]	name		Tablespace name
@param[in]	space_id	Tablespace ID
@param[in]	flags		Tablespace flags
@param[in]	purpose		Tablespace purpose
@return pointer to created tablespace, to be filled in with fil_node_create()
@retval nullptr on failure (such as when the same tablespace exists) */
fil_space_t *fil_space_create(const char *name, space_id_t space_id,
                              uint32_t flags, fil_type_t purpose) {
  ut_ad(fsp_flags_is_valid(flags));
  ut_ad(srv_page_size == UNIV_PAGE_SIZE_ORIG || flags != 0);

  DBUG_EXECUTE_IF("fil_space_create_failure", return (nullptr););

  if (purpose != FIL_TYPE_TEMPORARY) {
    /* Mark the clone as aborted only while executing a DDL which creates
    a base table, as any temporary table is ignored while cloning the database.
    Clone state must be set back to active before returning from function. */
    clone_mark_abort(true);
  }

  fil_system->mutex_acquire_all();

  auto shard = fil_system->shard_by_id(space_id);

  auto space = shard->space_create(name, space_id, flags, purpose);

  if (space == nullptr) {
    /* Duplicate error. */
    fil_system->mutex_release_all();

    if (purpose != FIL_TYPE_TEMPORARY) {
      clone_mark_active();
    }

    return (nullptr);
  }

  /* Cache the system tablespaces, avoid looking them up during IO. */

  if (space->id == TRX_SYS_SPACE) {
    ut_a(fil_space_t::s_sys_space == nullptr ||
         fil_space_t::s_sys_space == space);

    fil_space_t::s_sys_space = space;

  } else if (space->id == dict_sys_t_s_log_space_first_id) {
    ut_a(fil_space_t::s_redo_space == nullptr ||
         fil_space_t::s_redo_space == space);

    fil_space_t::s_redo_space = space;
  }

  fil_system->mutex_release_all();

  if (purpose != FIL_TYPE_TEMPORARY) {
    clone_mark_active();
  }

  return (space);
}
