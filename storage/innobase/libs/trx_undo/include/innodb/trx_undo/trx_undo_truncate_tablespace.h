#pragma once

#include <innodb/univ/univ.h>

/* Forward declaration. */
namespace undo {
struct Tablespace;
class Truncate;
}  // namespace undo

/** Truncate UNDO tablespace, reinitialize header and rseg.
@param[in]  marked_space  UNDO tablespace to truncate
@return true if success else false. */
bool trx_undo_truncate_tablespace(undo::Tablespace *marked_space);
