#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/page_no_t.h>

/** Find the page number in the TRX_SYS page for a given slot/rseg_id
@param[in]	rseg_id		slot number in the TRX_SYS page rseg array
@return page number from the TRX_SYS page rseg array */
page_no_t trx_sysf_rseg_find_page_no(ulint rseg_id);
