#pragma once

#include <innodb/univ/univ.h>

#include <innodb/page/page_zip_dir_start_offs.h>
#include <innodb/page/page_zip_t.h>

/** Gets a pointer to the compressed page trailer (the dense page directory),
 including deleted records (the free list).
 @param[in] page_zip compressed page
 @param[in] n_dense number of entries in the directory
 @return pointer to the dense page directory */
#define page_zip_dir_start_low(page_zip, n_dense) \
  ((page_zip)->data + page_zip_dir_start_offs(page_zip, n_dense))
