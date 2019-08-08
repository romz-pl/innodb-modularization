#pragma once

#include <innodb/univ/univ.h>

/** Rotate the redo log encryption
It will re-encrypt the redo log encryption metadata and write it to
redo log file header.
@return true if success. */
bool log_rotate_encryption();
