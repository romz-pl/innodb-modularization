#include <innodb/log_write/log_rotate_encryption.h>

#include <innodb/disk/flags.h>
#include <innodb/log_write/log_write_encryption.h>
#include <innodb/tablespace/consts.h>
#include <innodb/tablespace/fil_space_get.h>
#include <innodb/tablespace/fil_space_t.h>

bool log_rotate_encryption() {
  fil_space_t *space = fil_space_get(dict_sys_t_s_log_space_first_id);

  if (!FSP_FLAGS_GET_ENCRYPTION(space->flags)) {
    return (true);
  }

  /* Rotate log tablespace */
  return (log_write_encryption(nullptr, nullptr, false));
}
