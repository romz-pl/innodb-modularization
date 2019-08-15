#pragma once

#include <innodb/univ/univ.h>

struct dict_table_t;

/** Finds the highest [number] for foreign key constraints of the table. Looks
 only at the >= 4.0.18-format id's, which are of the form
 databasename/tablename_ibfk_[number].
 @return highest number, 0 if table has no new format foreign key constraints */
ulint dict_table_get_highest_foreign_id(
    dict_table_t *table); /*!< in: table in the dictionary
                          memory cache */
