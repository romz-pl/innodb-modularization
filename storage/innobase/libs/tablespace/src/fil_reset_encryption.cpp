#include <innodb/tablespace/fil_reset_encryption.h>

#include <innodb/tablespace/fil_system.h>
#include <innodb/tablespace/fsp_is_system_or_temp_tablespace.h>

/** Reset the encryption type for the tablespace
@param[in] space_id		Space ID of tablespace for which to set
@return DB_SUCCESS or error code */
dberr_t fil_reset_encryption(space_id_t space_id) {
  ut_ad(space_id != TRX_SYS_SPACE);

  if (fsp_is_system_or_temp_tablespace(space_id)) {
    return (DB_IO_NO_ENCRYPT_TABLESPACE);
  }

  auto shard = fil_system->shard_by_id(space_id);

  shard->mutex_acquire();

  fil_space_t *space = shard->get_space_by_id(space_id);

  if (space == NULL) {
    shard->mutex_release();
    return (DB_NOT_FOUND);
  }

  memset(space->encryption_key, 0, ENCRYPTION_KEY_LEN);
  space->encryption_klen = 0;

  memset(space->encryption_iv, 0, ENCRYPTION_KEY_LEN);

  space->encryption_type = Encryption::NONE;

  shard->mutex_release();

  return (DB_SUCCESS);
}
