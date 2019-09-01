#pragma once

#include <innodb/univ/univ.h>

struct lock_t;

/** Gets the first or next record lock on a page.
 @return next lock, NULL if none exists */
lock_t *lock_rec_get_next_on_page(lock_t *lock); /*!< in: a record lock */
