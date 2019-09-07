#include <innodb/trx_purge/spaces.h>

namespace undo {

/** A global object that contains a vector of undo::Tablespace structs. */
Tablespaces *spaces;

}
