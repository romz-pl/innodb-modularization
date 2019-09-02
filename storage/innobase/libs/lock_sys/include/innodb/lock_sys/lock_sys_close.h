#pragma once

#include <innodb/univ/univ.h>

/** Closes the lock system at database shutdown. */
void lock_sys_close(void);
