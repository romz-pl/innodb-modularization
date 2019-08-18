#include <innodb/page/page_zip_header_cmp.h>


#ifdef UNIV_DEBUG
/** Assert that the compressed and decompressed page headers match.
 @return true */
ibool page_zip_header_cmp(
    const page_zip_des_t *page_zip, /*!< in: compressed page */
    const byte *page)               /*!< in: uncompressed page */
{
  ut_ad(!memcmp(page_zip->data + FIL_PAGE_PREV, page + FIL_PAGE_PREV,
                FIL_PAGE_LSN - FIL_PAGE_PREV));
  ut_ad(!memcmp(page_zip->data + FIL_PAGE_TYPE, page + FIL_PAGE_TYPE, 2));
  ut_ad(!memcmp(page_zip->data + FIL_PAGE_DATA, page + FIL_PAGE_DATA,
                PAGE_DATA - FIL_PAGE_DATA));

  return (TRUE);
}
#endif /* UNIV_DEBUG */
