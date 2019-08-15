#pragma once

#include <innodb/univ/univ.h>

struct dict_table_t;

/** Get the innodb column position for a non-virtual column according to
its original MySQL table position n
@param[in]	table	table
@param[in]	n	MySQL column position
@return column position in InnoDB */
ulint dict_table_mysql_pos_to_innodb(const dict_table_t *table, ulint n);
