#pragma once

#include <innodb/univ/univ.h>

struct mtr_t;
struct page_zip_des_t;

/** Write data to the uncompressed header portion of a page. The data must
already have been written to the uncompressed page.
@param[in,out]	page_zip	compressed page
@param[in]	str		address on the uncompressed page
@param[in]	length		length of the data
@param[in]	mtr		mini-transaction, or NULL */
void page_zip_write_header(page_zip_des_t *page_zip, const byte *str,
                           ulint length, mtr_t *mtr);
