#pragma once

#include <innodb/univ/univ.h>

#include "sql/sql_class.h"

THD *thd_trx_arbitrate(THD *requestor, THD *holder);
