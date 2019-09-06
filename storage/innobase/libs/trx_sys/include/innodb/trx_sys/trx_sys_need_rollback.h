#pragma once

#include <innodb/univ/univ.h>

/** Determine if there are incomplete transactions in the system.
@return whether incomplete transactions need rollback */
bool trx_sys_need_rollback();
