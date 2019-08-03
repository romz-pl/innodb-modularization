#pragma once

#include <innodb/univ/univ.h>

/** Sets the file page type.
@param[in,out]	page		File page
@param[in]	type		File page type to set */
void fil_page_set_type(byte *page, ulint type);
