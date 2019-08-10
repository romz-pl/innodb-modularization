#pragma once

#include <innodb/univ/univ.h>

#include "sql/handler.h"

/** Fix cloned non-Innodb tables during recovery.
@param[in,out]  thd     current THD
@return true if error */
bool fix_cloned_tables(THD *thd);
