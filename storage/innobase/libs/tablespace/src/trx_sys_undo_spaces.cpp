#include <innodb/tablespace/trx_sys_undo_spaces.h>

/** A list of undo tablespace IDs found in the TRX_SYS page. These are the
old type of undo tablespaces that do not have space_IDs in the reserved
range nor contain an RSEG_ARRAY page. This cannot be part of the trx_sys_t
object because it must be built before that is initialized. */
Space_Ids *trx_sys_undo_spaces;
