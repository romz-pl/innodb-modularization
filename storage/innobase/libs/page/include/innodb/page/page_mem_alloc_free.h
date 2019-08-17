#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>
#include <innodb/univ/rec_t.h>

struct page_zip_des_t;

/** Allocates a block of memory from the head of the free list of an index
page.
@param[in,out]	page		index page
@param[in,out]	page_zip	compressed page with enough space available
                                for inserting the record, or NULL
@param[in]	next_rec	pointer to the new head of the free record
                                list
@param[in]	need		number of bytes allocated */
void page_mem_alloc_free(page_t *page, page_zip_des_t *page_zip,
                         rec_t *next_rec, ulint need);
