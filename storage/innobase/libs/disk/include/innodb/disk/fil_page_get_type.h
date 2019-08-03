#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_type_t.h>

/** Get the file page type.
@param[in]	page		File page
@return page type */
page_type_t fil_page_get_type(const byte *page);
