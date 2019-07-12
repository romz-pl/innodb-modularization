#include <innodb/tablespace/fsp_flags_are_equal.h>

#include <innodb/page/flag.h>

#define ACTUAL_SSIZE(ssize) (0 == ssize ? UNIV_PAGE_SSIZE_ORIG : ssize)

/** Determine if two tablespaces are equivalent or compatible.
@param[in]	flags1	First tablespace flags
@param[in]	flags2	Second tablespace flags
@return true the flags are compatible, false if not */
bool fsp_flags_are_equal(uint32_t flags1, uint32_t flags2) {
  /* If either one of these flags is UINT32_UNDEFINED,
  then they are not equal */
  if (flags1 == UINT32_UNDEFINED || flags2 == UINT32_UNDEFINED) {
    return (false);
  }

  if (!fsp_is_shared_tablespace(flags1) || !fsp_is_shared_tablespace(flags2)) {
    /* At least one of these is a single-table tablespaces so all
    flags must match. */
    return (flags1 == flags2);
  }

  /* Both are shared tablespaces which can contain all formats.
  But they must have the same logical and physical page size.
  Once InnoDB can support multiple page sizes together,
  the logical page size will not matter. */
  ulint zip_ssize1 = ACTUAL_SSIZE(FSP_FLAGS_GET_ZIP_SSIZE(flags1));
  ulint zip_ssize2 = ACTUAL_SSIZE(FSP_FLAGS_GET_ZIP_SSIZE(flags2));
  ulint page_ssize1 = ACTUAL_SSIZE(FSP_FLAGS_GET_PAGE_SSIZE(flags1));
  ulint page_ssize2 = ACTUAL_SSIZE(FSP_FLAGS_GET_PAGE_SSIZE(flags2));

  return (zip_ssize1 == zip_ssize2 && page_ssize1 == page_ssize2);
}
