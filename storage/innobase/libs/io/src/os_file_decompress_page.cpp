#include <innodb/io/os_file_decompress_page.h>

#include <innodb/io/Compression.h>

/** Decompress the page data contents. Page type must be FIL_PAGE_COMPRESSED, if
not then the source contents are left unchanged and DB_SUCCESS is returned.
@param[in]	dblwr_recover	true of double write recovery in progress
@param[in,out]	src		Data read from disk, decompressed data will be
                                copied to this page
@param[in,out]	dst		Scratch area to use for decompression
@param[in]	dst_len		Size of the scratch area in bytes
@return DB_SUCCESS or error code */
dberr_t os_file_decompress_page(bool dblwr_recover, byte *src, byte *dst,
                                ulint dst_len) {
  return (Compression::deserialize(dblwr_recover, src, dst, dst_len));
}
