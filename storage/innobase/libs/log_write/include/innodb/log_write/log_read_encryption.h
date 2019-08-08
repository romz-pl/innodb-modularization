#pragma once

#include <innodb/univ/univ.h>

/* Read the first log file header to get the encryption
information if it exist.
@return true if success */
bool log_read_encryption();
