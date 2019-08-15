#pragma once

#include <innodb/univ/univ.h>

/** Get the database name length in a table name.
 @return database name length */
ulint dict_get_db_name_len(const char *name) /*!< in: table name in the form
                                             dbname '/' tablename */
    MY_ATTRIBUTE((warn_unused_result));
