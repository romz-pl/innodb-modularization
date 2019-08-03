#pragma once

#include <innodb/univ/univ.h>

struct page_zip_des_t;

/** Set the size of a compressed page in bytes.
@param[in,out]	page_zip	compressed page
@param[in]	size		size in bytes */
void page_zip_set_size(page_zip_des_t *page_zip, ulint size);
