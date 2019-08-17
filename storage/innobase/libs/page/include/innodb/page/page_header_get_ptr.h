#pragma once

#include <innodb/univ/univ.h>

#include <innodb/page/page_header_get_offs.h>
#include <innodb/page/page_header_get_offs.h>

/** Returns the pointer stored in the given header field, or NULL. */
#define page_header_get_ptr(page, field)          \
  (page_header_get_offs(page, field)              \
       ? page + page_header_get_offs(page, field) \
       : NULL)
