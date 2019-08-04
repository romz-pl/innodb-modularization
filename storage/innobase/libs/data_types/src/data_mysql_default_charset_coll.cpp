#include <innodb/data_types/data_mysql_default_charset_coll.h>

/* At the database startup we store the default-charset collation number of
this MySQL installation to this global variable. If we have < 4.1.2 format
column definitions, or records in the insert buffer, we use this
charset-collation code for them. */

ulint data_mysql_default_charset_coll;
