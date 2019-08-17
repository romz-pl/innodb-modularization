#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>

/** Looks for the record which owns the given record.
 @return the owner record */
rec_t *page_rec_find_owner_rec(rec_t *rec); /*!< in: the physical record */
