#pragma once

#include <innodb/univ/univ.h>

/** If the last data file is auto-extended, we add this many pages to it
at a time. We have to make this public because it is a config variable. */
extern ulong sys_tablespace_auto_extend_increment;
