#pragma once

#include <innodb/univ/univ.h>

/** Return the end of table name where we have removed dbname and '/'.
 @return table name */
const char *dict_remove_db_name(
    const char *name) /*!< in: table name in the form
                      dbname '/' tablename */
    MY_ATTRIBUTE((warn_unused_result));
