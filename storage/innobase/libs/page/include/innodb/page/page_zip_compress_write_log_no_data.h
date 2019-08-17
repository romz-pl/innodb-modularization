#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

struct mtr_t;
struct dict_index_t;

/** Write a log record of compressing an index page without the data on the
page.
@param[in]	level	compression level
@param[in]	page	page that is compressed
@param[in]	index	index
@param[in]	mtr	mtr */
void page_zip_compress_write_log_no_data(ulint level, const page_t *page,
                                         dict_index_t *index, mtr_t *mtr);
