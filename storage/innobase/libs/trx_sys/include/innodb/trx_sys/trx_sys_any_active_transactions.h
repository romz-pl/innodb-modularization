#pragma once

#include <innodb/univ/univ.h>

/*
Check if there are any active (non-prepared) transactions.
@return total number of active transactions or 0 if none */
ulint trx_sys_any_active_transactions(void);
