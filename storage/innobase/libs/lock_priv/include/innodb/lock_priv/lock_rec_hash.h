#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/page_no_t.h>

/** Calculates the hash value of a page file address: used in inserting or
searching for a lock in the hash table.
@param[in]	space	space
@param[in]	page_no	page number
@return hashed value */
ulint lock_rec_hash(space_id_t space, page_no_t page_no);
