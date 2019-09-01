#pragma once

#include <innodb/univ/univ.h>
#include <innodb/univ/page_no_t.h>

struct lock_t;
struct hash_table_t;

/** Gets the first record lock on a page, where the page is identified by its
file address.
@param[in]	lock_hash	lock hash table
@param[in]	space		space
@param[in]	page_no		page number
@return first lock, NULL if none exists */
lock_t *lock_rec_get_first_on_page_addr(hash_table_t *lock_hash,
                                        space_id_t space, page_no_t page_no);
