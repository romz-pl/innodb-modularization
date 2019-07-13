#include <innodb/tablespace/fil_set_encryption.h>

#include <innodb/tablespace/fil_system.h>
#include <innodb/tablespace/fsp_is_system_or_temp_tablespace.h>

/** Set the encryption type for the tablespace
@param[in] space_id		Space ID of tablespace for which to set
@param[in] algorithm		Encryption algorithm
@param[in] key			Encryption key
@param[in] iv			Encryption iv
@return DB_SUCCESS or error code */
dberr_t fil_set_encryption(space_id_t space_id, Encryption::Type algorithm,
                           byte *key, byte *iv) {
  ut_ad(space_id != TRX_SYS_SPACE);

  if (fsp_is_system_or_temp_tablespace(space_id)) {
    return (DB_IO_NO_ENCRYPT_TABLESPACE);
  }

  auto shard = fil_system->shard_by_id(space_id);

  shard->mutex_acquire();

  fil_space_t *space = shard->get_space_by_id(space_id);

  if (space == nullptr) {
    shard->mutex_release();
    return (DB_NOT_FOUND);
  }

  if (key == nullptr) {
    Encryption::random_value(space->encryption_key);
  } else {
    memcpy(space->encryption_key, key, ENCRYPTION_KEY_LEN);
  }

  space->encryption_klen = ENCRYPTION_KEY_LEN;

  if (iv == nullptr) {
    Encryption::random_value(space->encryption_iv);
  } else {
    memcpy(space->encryption_iv, iv, ENCRYPTION_KEY_LEN);
  }

  ut_ad(algorithm != Encryption::NONE);
  space->encryption_type = algorithm;

  shard->mutex_release();

  return (DB_SUCCESS);
}

