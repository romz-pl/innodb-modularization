#pragma once

#include <innodb/univ/univ.h>

#include "sql/sql_class.h"

/**
@param[in] thd		Session to check
@return the priority */

int thd_trx_priority(THD *thd);
