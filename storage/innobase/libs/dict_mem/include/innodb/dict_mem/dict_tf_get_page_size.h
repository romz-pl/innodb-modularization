#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/flags.h>
#include <innodb/assert/assert.h>
#include <innodb/disk/page_size_t.h>
#include <innodb/disk/univ_page_size.h>

/** Extract the page size info from table flags.
@param[in]	flags	flags
@return a structure containing the compressed and uncompressed
page sizes and a boolean indicating if the page is compressed. */
UNIV_INLINE
const page_size_t dict_tf_get_page_size(uint32_t flags) {
  const ulint zip_ssize = DICT_TF_GET_ZIP_SSIZE(flags);

  if (zip_ssize == 0) {
    return (univ_page_size);
  }

  const ulint zip_size = (UNIV_ZIP_SIZE_MIN >> 1) << zip_ssize;

  ut_ad(zip_size <= UNIV_ZIP_SIZE_MAX);

  return (page_size_t(zip_size, univ_page_size.logical(), true));
}
