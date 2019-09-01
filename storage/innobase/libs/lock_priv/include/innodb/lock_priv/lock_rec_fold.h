#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/page_no_t.h>

/** Calculates the fold value of a page file address: used in inserting or
 searching for a lock in the hash table.
 @return folded value */
ulint lock_rec_fold(space_id_t space,  /*!< in: space */
                    page_no_t page_no) /*!< in: page number */
    MY_ATTRIBUTE((const));
