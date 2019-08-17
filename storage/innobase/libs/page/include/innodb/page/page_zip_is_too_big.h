#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

struct dict_index_t;
struct dtuple_t;

#ifndef UNIV_HOTBACKUP
/** Check whether a tuple is too big for compressed table
@param[in]	index	dict index object
@param[in]	entry	entry for the index
@return	true if it's too big, otherwise false */
bool page_zip_is_too_big(const dict_index_t *index, const dtuple_t *entry);
#endif /* !UNIV_HOTBACKUP */
