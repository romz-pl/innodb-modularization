#include <innodb/log_write/redo_rotate_default_master_key.h>

#include <innodb/disk/flags.h>
#include <innodb/io/Encryption.h>
#include <innodb/ioasync/srv_shutdown_state.h>
#include <innodb/log_write/log_write_encryption.h>
#include <innodb/tablespace/fil_space_get.h>
#include <innodb/tablespace/fil_space_t.h>
#include <innodb/tablespace/consts.h>
#include <innodb/io/srv_read_only_mode.h>

#include "sql/mysqld.h"

void redo_rotate_default_master_key() {
  fil_space_t *space = fil_space_get(dict_sys_t_s_log_space_first_id);

  if (srv_shutdown_state != SRV_SHUTDOWN_NONE) {
    return;
  }

  /* If the redo log space is using default key, rotate it.
  We also need the server_uuid initialized. */
  if (space->encryption_type != Encryption::NONE &&
      Encryption::s_master_key_id == ENCRYPTION_DEFAULT_MASTER_KEY_ID &&
      !srv_read_only_mode && strlen(server_uuid) > 0) {
    ut_a(FSP_FLAGS_GET_ENCRYPTION(space->flags));

    log_write_encryption(nullptr, nullptr, false);
  }
}
