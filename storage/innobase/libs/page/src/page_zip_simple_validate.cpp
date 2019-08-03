#include <innodb/page/page_zip_simple_validate.h>

#ifdef UNIV_DEBUG
/** Validate a compressed page descriptor.
 @return true if ok */
UNIV_INLINE
ibool page_zip_simple_validate(
    const page_zip_des_t *page_zip) /*!< in: compressed page descriptor */
{
  ut_ad(page_zip);
  ut_ad(page_zip->data);
  ut_ad(page_zip->ssize <= PAGE_ZIP_SSIZE_MAX);
  ut_ad(page_zip_get_size(page_zip) > PAGE_DATA + PAGE_ZIP_DIR_SLOT_SIZE);
  ut_ad(page_zip->m_start <= page_zip->m_end);
  ut_ad(page_zip->m_end < page_zip_get_size(page_zip));
  ut_ad(page_zip->n_blobs <
        page_zip_get_size(page_zip) / BTR_EXTERN_FIELD_REF_SIZE);
  return (TRUE);
}
#endif /* UNIV_DEBUG */

