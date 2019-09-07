#pragma once

#include <innodb/univ/univ.h>

#include <innodb/sync_mutex/ib_mutex_t.h>

namespace undo {

/** Mutext for serializing undo tablespace related DDL.  These have to do with
creating and dropping undo tablespaces. */
extern ib_mutex_t ddl_mutex;

}
