#pragma once

#include <innodb/univ/univ.h>

#include <innodb/page/fil_page_type_is_index.h>
#include <innodb/page/fil_page_get_type.h>

/** Check whether the page is index page (either regular Btree index or Rtree
index */
#define fil_page_index_page_check(page) \
  fil_page_type_is_index(fil_page_get_type(page))
