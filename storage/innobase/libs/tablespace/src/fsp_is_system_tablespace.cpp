#include <innodb/tablespace/fsp_is_system_tablespace.h>

#include <innodb/tablespace/consts.h>
#include <innodb/trx_sys/flags.h>

bool fsp_is_system_tablespace(space_id_t space_id) {
  return (space_id == TRX_SYS_SPACE);
}
