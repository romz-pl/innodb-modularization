#include <innodb/tablespace/fsp_header_get_encryption_progress_offset.h>

#include <innodb/page/page_size_t.h>
#include <innodb/tablespace/fsp_header_get_sdi_offset.h>
#include <innodb/tablespace/consts.h>

/** Get the offset of encrytion progress information in page 0.
@param[in]      page_size       page size.
@return offset on success, otherwise 0. */
ulint fsp_header_get_encryption_progress_offset(
    const page_size_t &page_size) {
  ulint offset;
#ifdef UNIV_DEBUG
  ulint left_size;
#endif

  offset = fsp_header_get_sdi_offset(page_size) + FSP_SDI_HEADER_LEN;
#ifdef UNIV_DEBUG
  left_size =
      page_size.physical() - FSP_HEADER_OFFSET - offset - FIL_PAGE_DATA_END;

  ut_ad(left_size >=
        ENCRYPTION_OPERATION_INFO_SIZE + ENCRYPTION_PROGRESS_INFO_SIZE);
#endif

  return (offset);
}
