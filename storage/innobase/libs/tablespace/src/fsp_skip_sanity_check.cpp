#include <innodb/tablespace/fsp_skip_sanity_check.h>


#ifndef UNIV_HOTBACKUP
#ifdef UNIV_DEBUG

/** Skip some of the sanity checks that are time consuming even in debug mode
and can affect frequent verification runs that are done to ensure stability of
the product.
@return true if check should be skipped for given space. */
bool fsp_skip_sanity_check(space_id_t space_id) {
  return (srv_skip_temp_table_checks_debug &&
          fsp_is_system_temporary(space_id));
}

#endif /* UNIV_DEBUG */
#endif /* UNIV_HOTBACKUP */
