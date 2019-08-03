#pragma once

#include <innodb/univ/univ.h>

#include <innodb/page/page_zip_dir_elems.h>
#include <innodb/page/page_zip_dir_start_low.h>

/** Gets a pointer to the compressed page trailer (the dense page directory),
 including deleted records (the free list).
 @param[in] page_zip compressed page
 @return pointer to the dense page directory */
#define page_zip_dir_start(page_zip) \
  page_zip_dir_start_low(page_zip, page_zip_dir_elems(page_zip))
