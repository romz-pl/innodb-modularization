#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

struct page_zip_des_t;

/** Sets the given header field.
@param[in,out]	page		page
@param[in,out]	page_zip	compressed page whose uncompressed part will
                                be updated, or NULL
@param[in]	field		PAGE_N_DIR_SLOTS, ...
@param[in]	val		value */
void page_header_set_field(page_t *page, page_zip_des_t *page_zip, ulint field,
                           ulint val);
