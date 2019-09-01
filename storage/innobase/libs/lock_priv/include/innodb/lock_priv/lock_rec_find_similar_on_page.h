#pragma once

#include <innodb/univ/univ.h>

struct lock_t;
struct trx_t;

/** Looks for a suitable type record lock struct by the same trx on the same
page. This can be used to save space when a new record lock should be set on a
page: no new struct is needed, if a suitable old is found.
@param[in]	type_mode	lock type_mode field
@param[in]	heap_no		heap number of the record
@param[in]	lock		lock_rec_get_first_on_page()
@param[in]	trx		transaction
@return lock or NULL */
UNIV_INLINE
lock_t *lock_rec_find_similar_on_page(ulint type_mode, ulint heap_no,
                                      lock_t *lock, const trx_t *trx);
