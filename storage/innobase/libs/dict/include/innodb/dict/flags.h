#pragma once

#include <innodb/univ/univ.h>


/** Prefix for tmp tables, adopted from sql/table.h */
#define TEMP_FILE_PREFIX "#sql"
#define TEMP_FILE_PREFIX_LENGTH 4
#define TEMP_FILE_PREFIX_INNODB "#sql-ib"

#define TEMP_TABLE_PREFIX "#sql"
#define TEMP_TABLE_PATH_PREFIX "/" TEMP_TABLE_PREFIX
