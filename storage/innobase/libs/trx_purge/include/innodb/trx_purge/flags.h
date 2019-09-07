#pragma once

#include <innodb/univ/univ.h>


/* Namespace to hold all the related functions and variables needed
to truncate an undo tablespace. */
namespace undo {

/** Magic Number to indicate truncate action is complete. */
const ib_uint32_t s_magic = 76845412;

/** Truncate Log file Prefix. */
const char *const s_log_prefix = "undo_";

/** Truncate Log file Extension. */
const char *const s_log_ext = "trunc.log";


}
