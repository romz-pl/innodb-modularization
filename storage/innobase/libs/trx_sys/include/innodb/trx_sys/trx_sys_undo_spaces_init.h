#pragma once

#include <innodb/univ/univ.h>

/** Initialize trx_sys_undo_spaces, called once during srv_start(). */
void trx_sys_undo_spaces_init();
