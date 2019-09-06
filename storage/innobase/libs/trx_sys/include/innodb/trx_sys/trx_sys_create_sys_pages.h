#pragma once

#include <innodb/univ/univ.h>

/** Creates and initializes the transaction system at the database creation. */
void trx_sys_create_sys_pages(void);
