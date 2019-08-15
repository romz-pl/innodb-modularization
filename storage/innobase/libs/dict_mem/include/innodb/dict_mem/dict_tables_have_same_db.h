#pragma once

#include <innodb/univ/univ.h>

/** Checks if the database name in two table names is the same.
 @return true if same db name */
ibool dict_tables_have_same_db(
    const char *name1, /*!< in: table name in the form
                       dbname '/' tablename */
    const char *name2) /*!< in: table name in the form
                       dbname '/' tablename */
    MY_ATTRIBUTE((warn_unused_result));
