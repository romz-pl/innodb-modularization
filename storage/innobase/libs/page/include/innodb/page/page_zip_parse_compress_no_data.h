#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>


struct mtr_t;
struct page_zip_des_t;
struct dict_index_t;

/** Parses a log record of compressing an index page without the data.
@param[in]	ptr		buffer
@param[in]	end_ptr		buffer end
@param[in]	page		uncompressed page
@param[out]	page_zip	compressed page
@param[in]	index		index
@return end of log record or NULL */
byte *page_zip_parse_compress_no_data(byte *ptr, byte *end_ptr, page_t *page,
                                      page_zip_des_t *page_zip,
                                      dict_index_t *index);
