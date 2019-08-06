#include <innodb/tablespace/fsp_flags_set_page_size.h>

#include <innodb/disk/page_size_t.h>
#include <innodb/page/page_size_to_ssize.h>
#include <innodb/tablespace/fsp_flags_is_valid.h>

/** Add the page size to the tablespace flags.
@param[in]	flags		Tablespace flags
@param[in]	page_size	page sizes in bytes and compression flag.
@return tablespace flags after page size is added */
uint32_t fsp_flags_set_page_size(uint32_t flags, const page_size_t &page_size) {
  /* Page size should be a power of two between UNIV_PAGE_SIZE_MIN
  and UNIV_PAGE_SIZE */
  ut_ad(page_size.logical() >= UNIV_PAGE_SIZE_MIN);
  ut_ad(page_size.logical() <= UNIV_PAGE_SIZE_MAX);
  ut_ad(ut_is_2pow(page_size.logical()));

  /* Remove this assert once we add support for different
  page size per tablespace. Currently all tablespaces must
  have a page size that is equal to innodb-page-size */
  ut_ad(page_size.logical() == UNIV_PAGE_SIZE);

  if (page_size.logical() == UNIV_PAGE_SIZE_ORIG) {
    ut_ad(0 == FSP_FLAGS_GET_PAGE_SSIZE(flags));

  } else {
    uint32_t ssize = page_size_to_ssize(page_size.logical());

    ut_ad(ssize);
    ut_ad(ssize <= UNIV_PAGE_SSIZE_MAX);

    flags |= (ssize << FSP_FLAGS_POS_PAGE_SSIZE);
  }

  ut_ad(fsp_flags_is_valid(flags));

  return (flags);
}
