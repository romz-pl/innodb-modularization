#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

struct page_zip_des_t;
struct mtr_t;

#ifndef UNIV_HOTBACKUP

/** Resets the last insert info field in the page header. Writes to mlog about
this operation.
@param[in]	page		page
@param[in,out]	page_zip	compressed page whose uncompressed part will
                                be updated, or NULL
@param[in]	mtr		mtr */
void page_header_reset_last_insert(page_t *page, page_zip_des_t *page_zip,
                                   mtr_t *mtr);
#endif /* !UNIV_HOTBACKUP */
