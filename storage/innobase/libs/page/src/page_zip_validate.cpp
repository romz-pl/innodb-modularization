#include <innodb/page/page_zip_validate.h>

#ifdef UNIV_ZIP_DEBUG

/** Check that the compressed and decompressed pages match.
 @return true if valid, false if not */
ibool page_zip_validate(
    const page_zip_des_t *page_zip, /*!< in: compressed page */
    const page_t *page,             /*!< in: uncompressed page */
    const dict_index_t *index)      /*!< in: index of the page, if known */
{
  return (page_zip_validate_low(page_zip, page, index, recv_recovery_is_on()));
}

#endif
