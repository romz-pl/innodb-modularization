#pragma once

#include <innodb/univ/univ.h>

class page_id_t;

/** Checks if a page address is the trx sys header page.
@param[in]	page_id	page id
@return true if trx sys header page */
bool trx_sys_hdr_page(const page_id_t &page_id);
