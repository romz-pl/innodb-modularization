#include <innodb/tablespace/sys_tablespace_auto_extend_increment.h>

/** If the last data file is auto-extended, we add this many pages to it
at a time. We have to make this public because it is a config variable. */
ulong sys_tablespace_auto_extend_increment;
