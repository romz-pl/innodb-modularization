#pragma once

#include <innodb/univ/univ.h>

/** Returns true if recovery is currently running.
@return recv_recovery_on */
bool recv_recovery_is_on();
