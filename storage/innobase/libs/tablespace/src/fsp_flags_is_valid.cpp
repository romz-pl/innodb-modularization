#include <innodb/tablespace/fsp_flags_is_valid.h>

#include <innodb/page/page_zip_t.h>
#include <innodb/page/flag.h>

/** Validate the tablespace flags.
These flags are stored in the tablespace header at offset FSP_SPACE_FLAGS.
They should be 0 for ROW_FORMAT=COMPACT and ROW_FORMAT=REDUNDANT.
The newer row formats, COMPRESSED and DYNAMIC, will have at least
the DICT_TF_COMPACT bit set.
@param[in]	flags	Tablespace flags
@return true if valid, false if not */
bool fsp_flags_is_valid(uint32_t flags) {
  bool post_antelope = FSP_FLAGS_GET_POST_ANTELOPE(flags);
  ulint zip_ssize = FSP_FLAGS_GET_ZIP_SSIZE(flags);
  bool atomic_blobs = FSP_FLAGS_HAS_ATOMIC_BLOBS(flags);
  ulint page_ssize = FSP_FLAGS_GET_PAGE_SSIZE(flags);
  bool has_data_dir = FSP_FLAGS_HAS_DATA_DIR(flags);
  bool is_shared = FSP_FLAGS_GET_SHARED(flags);
  bool is_temp = FSP_FLAGS_GET_TEMPORARY(flags);
  bool is_encryption = FSP_FLAGS_GET_ENCRYPTION(flags);

  ulint unused = FSP_FLAGS_GET_UNUSED(flags);

  DBUG_EXECUTE_IF("fsp_flags_is_valid_failure", return (false););

  /* The Antelope row formats REDUNDANT and COMPACT did
  not use tablespace flags, so the entire 4-byte field
  is zero for Antelope row formats. */
  if (flags == 0) {
    return (true);
  }

  /* Row_FORMAT=COMPRESSED and ROW_FORMAT=DYNAMIC use a feature called
  ATOMIC_BLOBS which builds on the page structure introduced for the
  COMPACT row format by allowing long fields to be broken into prefix
  and externally stored parts. So if it is Post_antelope, it uses
  Atomic BLOBs. */
  if (post_antelope != atomic_blobs) {
    return (false);
  }

  /* Make sure there are no bits that we do not know about. */
  if (unused != 0) {
    return (false);
  }

  /* The zip ssize can be zero if it is other than compressed row format,
  or it could be from 1 to the max. */
  if (zip_ssize > PAGE_ZIP_SSIZE_MAX) {
    return (false);
  }

  /* The actual page size must be within 4k and 64K (3 =< ssize =< 7). */
  if (page_ssize != 0 &&
      (page_ssize < UNIV_PAGE_SSIZE_MIN || page_ssize > UNIV_PAGE_SSIZE_MAX)) {
    return (false);
  }

  /* Only single-table tablespaces use the DATA DIRECTORY clause.
  It is not compatible with the TABLESPACE clause.  Nor is it
  compatible with the TEMPORARY clause. */
  if (has_data_dir && (is_shared || is_temp)) {
    return (false);
  }

  /* Only single-table and general tablespaces and not temp tablespaces
  use the encryption clause. */
  if (is_encryption && (is_temp)) {
    return (false);
  }

#if FSP_FLAGS_POS_UNUSED != 15
#error You have added a new FSP_FLAG without adding a validation check.
#endif

  return (true);
}
