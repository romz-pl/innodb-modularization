#include <innodb/tablespace/fsp_flags_init.h>

#include <innodb/disk/page_size_t.h>
#include <innodb/tablespace/fsp_flags_set_page_size.h>
#include <innodb/tablespace/fsp_flags_set_zip_size.h>

/** Initialize an FSP flags integer.
@param[in]	page_size	page sizes in bytes and compression flag.
@param[in]	atomic_blobs	Used by Dynammic and Compressed.
@param[in]	has_data_dir	This tablespace is in a remote location.
@param[in]	is_shared	This tablespace can be shared by many tables.
@param[in]	is_temporary	This tablespace is temporary.
@param[in]	is_encrypted	This tablespace is encrypted.
@return tablespace flags after initialization */
uint32_t fsp_flags_init(const page_size_t &page_size, bool atomic_blobs,
                        bool has_data_dir, bool is_shared, bool is_temporary,
                        bool is_encrypted) {
  ut_ad(page_size.physical() <= page_size.logical());
  ut_ad(!page_size.is_compressed() || atomic_blobs);

  /* Page size should be a power of two between UNIV_PAGE_SIZE_MIN
  and UNIV_PAGE_SIZE, but zip_size may be 0 if not compressed. */
  uint32_t flags = fsp_flags_set_page_size(0, page_size);

  if (atomic_blobs) {
    flags |= FSP_FLAGS_MASK_POST_ANTELOPE | FSP_FLAGS_MASK_ATOMIC_BLOBS;
  }

  /* If the zip_size is explicit and different from the default,
  compressed row format is implied. */
  flags = fsp_flags_set_zip_size(flags, page_size);

  if (has_data_dir) {
    flags |= FSP_FLAGS_MASK_DATA_DIR;
  }

  /* Shared tablespaces can hold all row formats, so we only mark the
  POST_ANTELOPE and ATOMIC_BLOB bits if it is compressed. */
  if (is_shared) {
    ut_ad(!has_data_dir);
    flags |= FSP_FLAGS_MASK_SHARED;
  }

  if (is_temporary) {
    ut_ad(!has_data_dir);
    flags |= FSP_FLAGS_MASK_TEMPORARY;
  }

  if (is_encrypted) {
    flags |= FSP_FLAGS_MASK_ENCRYPTION;
  }

  return (flags);
}
