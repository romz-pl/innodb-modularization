#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

/** Reads the given header field. */
ulint page_header_get_field(const page_t *page, /*!< in: page */
                            ulint field);        /*!< in: PAGE_LEVEL, ... */
