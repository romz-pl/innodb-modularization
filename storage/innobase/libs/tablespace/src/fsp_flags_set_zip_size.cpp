#include <innodb/tablespace/fsp_flags_set_zip_size.h>

#include <innodb/disk/page_size_t.h>
#include <innodb/page/page_size_to_ssize.h>
#include <innodb/tablespace/fsp_flags_is_valid.h>

/** Add the compressed page size to the tablespace flags.
@param[in]	flags		Tablespace flags
@param[in]	page_size	page sizes in bytes and compression flag.
@return tablespace flags after zip size is added */
uint32_t fsp_flags_set_zip_size(uint32_t flags, const page_size_t &page_size) {
  if (!page_size.is_compressed()) {
    return (flags);
  }

  /* Zip size should be a power of 2 between UNIV_ZIP_SIZE_MIN
  and UNIV_ZIP_SIZE_MAX */
  ut_ad(page_size.physical() >= UNIV_ZIP_SIZE_MIN);
  ut_ad(page_size.physical() <= UNIV_ZIP_SIZE_MAX);
  ut_ad(ut_is_2pow(page_size.physical()));

  uint32_t ssize = page_size_to_ssize(page_size.physical());

  ut_ad(ssize > 0);
  ut_ad(ssize <= UNIV_PAGE_SSIZE_MAX);

  flags |= (ssize << FSP_FLAGS_POS_ZIP_SSIZE);

  ut_ad(fsp_flags_is_valid(flags));

  return (flags);
}
