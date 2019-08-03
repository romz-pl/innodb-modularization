#include <innodb/tablespace/fsp_header_get_sdi_offset.h>

#include <innodb/disk/page_size_t.h>
#include <innodb/io/Encryption.h>
#include <innodb/tablespace/xdes_arr_size.h>
#include <innodb/tablespace/consts.h>

/** Get the offset of SDI root page number in page 0.
@param[in]	page_size	page size.
@return	offset on success, otherwise 0. */
ulint fsp_header_get_sdi_offset(const page_size_t &page_size) {
  ulint offset;
#ifdef UNIV_DEBUG
  ulint left_size;
#endif

  offset = XDES_ARR_OFFSET + XDES_SIZE * xdes_arr_size(page_size) +
           ENCRYPTION_INFO_MAX_SIZE;
#ifdef UNIV_DEBUG
  left_size =
      page_size.physical() - FSP_HEADER_OFFSET - offset - FIL_PAGE_DATA_END;

  ut_ad(left_size >= FSP_SDI_HEADER_LEN);
#endif
  return (offset);
}
