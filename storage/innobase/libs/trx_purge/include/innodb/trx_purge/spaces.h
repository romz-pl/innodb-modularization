#pragma once

#include <innodb/univ/univ.h>

#include <innodb/trx_purge/Tablespaces.h>

namespace undo {

/** A global object that contains a vector of undo::Tablespace structs. */
extern Tablespaces *spaces;

}
