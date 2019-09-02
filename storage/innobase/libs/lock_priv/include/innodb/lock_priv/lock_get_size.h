#pragma once

#include <innodb/univ/univ.h>

/** Gets the size of a lock struct.
 @return size in bytes */
ulint lock_get_size(void);
