#pragma once

#include <innodb/univ/univ.h>

#ifdef UNIV_DEBUG
/** Validate the trx_sys_t::rw_trx_list.
 @return true if the list is valid */
bool trx_sys_validate_trx_list();
#endif /* UNIV_DEBUG */
