#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_type_t.h>

/** Check whether the page type is index (Btree or Rtree or SDI) type */
#define fil_page_type_is_index(page_type)                      \
  (page_type == FIL_PAGE_INDEX || page_type == FIL_PAGE_SDI || \
   page_type == FIL_PAGE_RTREE)
