#include <innodb/tablespace/fil_space_t.h>

/** System tablespace. */
fil_space_t *fil_space_t::s_sys_space;

/** Redo log tablespace */
fil_space_t *fil_space_t::s_redo_space;
