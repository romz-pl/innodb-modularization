#include <innodb/io/os_file_compress_page.h>

#include <innodb/assert/assert.h>
#include <innodb/machine/data.h>
#include <innodb/page/page_type_t.h>
#include <innodb/page/type.h>
#include <innodb/error/ut_error.h>
#include <innodb/align/ut_calc_align.h>

#include <string.h>
#include <lz4.h>
#include <zlib.h>

/* Compression level to be used by zlib. Settable by user. */
extern uint page_zip_level;

/** Compress a data page
@param[in]	compression	Compression algorithm
@param[in]	block_size	File system block size
@param[in]	src		Source contents to compress
@param[in]	src_len		Length in bytes of the source
@param[out]	dst		Compressed page contents
@param[out]	dst_len		Length in bytes of dst contents
@return buffer data, dst_len will have the length of the data */
byte *os_file_compress_page(Compression compression, ulint block_size,
                                   byte *src, ulint src_len, byte *dst,
                                   ulint *dst_len) {
  ulint len = 0;
  ulint compression_level = page_zip_level;
  ulint page_type = mach_read_from_2(src + FIL_PAGE_TYPE);

  /* The page size must be a multiple of the OS punch hole size. */
  ut_ad(!(src_len % block_size));

  /* Shouldn't compress an already compressed page. */
  ut_ad(page_type != FIL_PAGE_COMPRESSED);

  /* The page must be at least twice as large as the file system
  block size if we are to save any space. Ignore R-Tree pages for now,
  they repurpose the same 8 bytes in the page header. No point in
  compressing if the file system block size >= our page size. */

  if (page_type == FIL_PAGE_RTREE || block_size == ULINT_UNDEFINED ||
      compression.m_type == Compression::NONE || src_len < block_size * 2) {
    *dst_len = src_len;

    return (src);
  }

  /* Leave the header alone when compressing. */
  ut_ad(block_size >= FIL_PAGE_DATA * 2);

  ut_ad(src_len > FIL_PAGE_DATA + block_size);

  /* Must compress to <= N-1 FS blocks. */
  ulint out_len = src_len - (FIL_PAGE_DATA + block_size);

  /* This is the original data page size - the page header. */
  ulint content_len = src_len - FIL_PAGE_DATA;

  ut_ad(out_len >= block_size - FIL_PAGE_DATA);
  ut_ad(out_len <= src_len - (block_size + FIL_PAGE_DATA));

  /* Only compress the data + trailer, leave the header alone */

  switch (compression.m_type) {
    case Compression::NONE:
      ut_error;

    case Compression::ZLIB: {
      uLongf zlen = static_cast<uLongf>(out_len);

      if (compress2(dst + FIL_PAGE_DATA, &zlen, src + FIL_PAGE_DATA,
                    static_cast<uLong>(content_len),
                    static_cast<int>(compression_level)) != Z_OK) {
        *dst_len = src_len;

        return (src);
      }

      len = static_cast<ulint>(zlen);

      break;
    }

    case Compression::LZ4:

      len = LZ4_compress_default(reinterpret_cast<char *>(src) + FIL_PAGE_DATA,
                                 reinterpret_cast<char *>(dst) + FIL_PAGE_DATA,
                                 static_cast<int>(content_len),
                                 static_cast<int>(out_len));

      ut_a(len <= src_len - FIL_PAGE_DATA);

      if (len == 0 || len >= out_len) {
        *dst_len = src_len;

        return (src);
      }

      break;

    default:
      *dst_len = src_len;
      return (src);
  }

  ut_a(len <= out_len);

  ut_ad(memcmp(src + FIL_PAGE_LSN + 4,
               src + src_len - FIL_PAGE_END_LSN_OLD_CHKSUM + 4, 4) == 0);

  /* Copy the header as is. */
  memmove(dst, src, FIL_PAGE_DATA);

  /* Add compression control information. Required for decompressing. */
  mach_write_to_2(dst + FIL_PAGE_TYPE, FIL_PAGE_COMPRESSED);

  mach_write_to_1(dst + FIL_PAGE_VERSION, 1);

  mach_write_to_1(dst + FIL_PAGE_ALGORITHM_V1, compression.m_type);

  mach_write_to_2(dst + FIL_PAGE_ORIGINAL_TYPE_V1, page_type);

  mach_write_to_2(dst + FIL_PAGE_ORIGINAL_SIZE_V1, content_len);

  mach_write_to_2(dst + FIL_PAGE_COMPRESS_SIZE_V1, len);

  /* Round to the next full block size */

  len += FIL_PAGE_DATA;

  *dst_len = ut_calc_align(len, block_size);

  ut_ad(*dst_len >= len && *dst_len <= out_len + FIL_PAGE_DATA);

  /* Clear out the unused portion of the page. */
  if (len % block_size) {
    memset(dst + len, 0x0, block_size - (len % block_size));
  }

  return (dst);
}
