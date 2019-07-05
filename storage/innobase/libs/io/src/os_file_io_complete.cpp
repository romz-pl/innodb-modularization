#include <innodb/io/os_file_io_complete.h>

#include <innodb/io/os_file_punch_hole.h>
#include <innodb/page/type.h>


dberr_t os_file_decompress_page(bool dblwr_recover, byte *src, byte *dst, ulint dst_len);

/** Decompress after a read and punch a hole in the file if it was a write
@param[in]	type		IO context
@param[in]	fh		Open file handle
@param[in,out]	buf		Buffer to transform
@param[in,out]	scratch		Scratch area for read decompression
@param[in]	src_len		Length of the buffer before compression
@param[in]	offset		file offset from the start where to read
@param[in]	len		Used buffer length for write and output
                                buf len for read
@return DB_SUCCESS or error code */
dberr_t os_file_io_complete(const IORequest &type, os_file_t fh,
                                   byte *buf, byte *scratch, ulint src_len,
                                   os_offset_t offset, ulint len) {
  dberr_t ret = DB_SUCCESS;

  /* We never compress/decompress the first page */
  ut_a(offset > 0);
  ut_ad(type.validate());

  if (!type.is_compression_enabled()) {
    if (type.is_log() && offset >= LOG_FILE_HDR_SIZE) {
      Encryption encryption(type.encryption_algorithm());

      ret = encryption.decrypt_log(type, buf, src_len, scratch, len);
    }

    return (ret);
  } else if (type.is_read()) {
    Encryption encryption(type.encryption_algorithm());

    ret = encryption.decrypt(type, buf, src_len, scratch, len);
    if (ret == DB_SUCCESS) {
      return (
          os_file_decompress_page(type.is_dblwr_recover(), buf, scratch, len));
    } else {
      return (ret);
    }
  } else if (type.punch_hole()) {
    ut_ad(len <= src_len);
    ut_ad(!type.is_log());
    ut_ad(type.is_write());
    ut_ad(type.is_compressed());

    /* Nothing to do. */
    if (len == src_len) {
      return (DB_SUCCESS);
    }

#ifdef UNIV_DEBUG
    const ulint block_size = type.block_size();
#endif /* UNIV_DEBUG */

    /* We don't support multiple page sizes in the server
    at the moment. */
    ut_ad(src_len == srv_page_size);

    /* Must be a multiple of the compression unit size. */
    ut_ad((len % block_size) == 0);
    ut_ad((offset % block_size) == 0);

    ut_ad(len + block_size <= src_len);

    offset += len;

    return (os_file_punch_hole(fh, offset, src_len - len));
  }

  ut_ad(!type.is_log());

  return (DB_SUCCESS);
}


