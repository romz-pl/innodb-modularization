#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

struct page_zip_des_t;

/** Sets the pointer stored in the given header field.
@param[in,out]	page		page
@param[in,out]	page_zip	compressed page whose uncompressed part will
                                be updated, or NULL
@param[in,out]	field		PAGE_FREE, ...
@param[in]	ptr		pointer or NULL */
void page_header_set_ptr(page_t *page, page_zip_des_t *page_zip, ulint field,
                         const byte *ptr);
