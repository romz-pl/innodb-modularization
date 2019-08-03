#include <innodb/io/Compression.h>

#include <innodb/allocator/ut_free.h>
#include <innodb/allocator/ut_malloc_nokey.h>
#include <innodb/assert/assert.h>
#include <innodb/logger/error.h>
#include <innodb/machine/data.h>
#include <innodb/disk/page_type_t.h>
#include <innodb/disk/flags.h>

#include <sstream>
#include <lz4.h>
#include <zlib.h>

/** @return true if it is a compressed page */
bool Compression::is_compressed_page(const byte *page) {
  return (mach_read_from_2(page + FIL_PAGE_TYPE) == FIL_PAGE_COMPRESSED);
}

/**
@param[in]      type            The compression type
@return the string representation */
const char *Compression::to_string(Type type) {
  switch (type) {
    case NONE:
      return ("None");
    case ZLIB:
      return ("Zlib");
    case LZ4:
      return ("LZ4");
  }

  ut_ad(0);

  return ("<UNKNOWN>");
}

/**
@param[in]      meta		Page Meta data
@return the string representation */
std::string Compression::to_string(const Compression::meta_t &meta) {
  std::ostringstream stream;

  stream << "version: " << int(meta.m_version) << " "
         << "algorithm: " << meta.m_algorithm << " "
         << "(" << to_string(meta.m_algorithm) << ") "
         << "orginal_type: " << meta.m_original_type << " "
         << "original_size: " << meta.m_original_size << " "
         << "compressed_size: " << meta.m_compressed_size;

  return (stream.str());
}

/** Deserizlise the page header compression meta-data
@param[in]	page		Pointer to the page header
@param[out]	control		Deserialised data */
void Compression::deserialize_header(const byte *page,
                                     Compression::meta_t *control) {
  ut_ad(is_compressed_page(page));

  control->m_version =
      static_cast<uint8_t>(mach_read_from_1(page + FIL_PAGE_VERSION));

  control->m_original_type =
      static_cast<uint16_t>(mach_read_from_2(page + FIL_PAGE_ORIGINAL_TYPE_V1));

  control->m_compressed_size =
      static_cast<uint16_t>(mach_read_from_2(page + FIL_PAGE_COMPRESS_SIZE_V1));

  control->m_original_size =
      static_cast<uint16_t>(mach_read_from_2(page + FIL_PAGE_ORIGINAL_SIZE_V1));

  control->m_algorithm =
      static_cast<Type>(mach_read_from_1(page + FIL_PAGE_ALGORITHM_V1));
}


/** Decompress the page data contents. Page type must be FIL_PAGE_COMPRESSED, if
not then the source contents are left unchanged and DB_SUCCESS is returned.
@param[in]	dblwr_recover	true of double write recovery in progress
@param[in,out]	src		Data read from disk, decompressed data will be
                                copied to this page
@param[in,out]	dst		Scratch area to use for decompression
@param[in]	dst_len		Size of the scratch area in bytes
@return DB_SUCCESS or error code */
dberr_t Compression::deserialize(bool dblwr_recover, byte *src, byte *dst,
                                 ulint dst_len) {
  if (!is_compressed_page(src)) {
    /* There is nothing we can do. */
    return (DB_SUCCESS);
  }

  meta_t header;

  deserialize_header(src, &header);

  byte *ptr = src + FIL_PAGE_DATA;

  if (header.m_version != 1 ||
      header.m_original_size < UNIV_PAGE_SIZE_MIN - (FIL_PAGE_DATA + 8) ||
      header.m_original_size > UNIV_PAGE_SIZE_MAX - FIL_PAGE_DATA ||
      dst_len < header.m_original_size + FIL_PAGE_DATA) {
    /* The last check could potentially return DB_OVERFLOW,
    the caller should be able to retry with a larger buffer. */

    return (DB_CORRUPTION);
  }

  // FIXME: We should use TLS for this and reduce the malloc/free
  bool allocated;

  /* The caller doesn't know what to expect */
  if (dst == NULL) {
    /* Add a safety margin of an additional 50% */
    ulint n_bytes = header.m_original_size + (header.m_original_size / 2);

    dst = reinterpret_cast<byte *>(ut_malloc_nokey(n_bytes));

    if (dst == NULL) {
      return (DB_OUT_OF_MEMORY);
    }

    allocated = true;
  } else {
    allocated = false;
  }

  int ret;
  Compression compression;
  ulint len = header.m_original_size;

  compression.m_type = static_cast<Compression::Type>(header.m_algorithm);

  switch (compression.m_type) {
    case Compression::ZLIB: {
      uLongf zlen = header.m_original_size;

      if (uncompress(dst, &zlen, ptr, header.m_compressed_size) != Z_OK) {
        if (allocated) {
          ut_free(dst);
        }

        return (DB_IO_DECOMPRESS_FAIL);
      }

      len = static_cast<ulint>(zlen);

      break;
    }

    case Compression::LZ4:

      if (dblwr_recover) {
        ret = LZ4_decompress_safe(
            reinterpret_cast<char *>(ptr), reinterpret_cast<char *>(dst),
            header.m_compressed_size, header.m_original_size);

      } else {
        /* This can potentially read beyond the input
        buffer if the data is malformed. According to
        the LZ4 documentation it is a little faster
        than the above function. When recovering from
        the double write buffer we can afford to us the
        slower function above. */

        ret = LZ4_decompress_fast(reinterpret_cast<char *>(ptr),
                                  reinterpret_cast<char *>(dst),
                                  header.m_original_size);
      }

      if (ret < 0) {
        if (allocated) {
          ut_free(dst);
        }

        return (DB_IO_DECOMPRESS_FAIL);
      }

      break;

    default:
#ifdef UNIV_NO_ERR_MSGS
      ib::error()
#else
      ib::error(ER_IB_MSG_741)
#endif /* UNIV_NO_ERR_MSGS */
          << "Compression algorithm support missing: "
          << Compression::to_string(compression.m_type);

      if (allocated) {
        ut_free(dst);
      }

      return (DB_UNSUPPORTED);
  }

  /* Leave the header alone */
  memmove(src + FIL_PAGE_DATA, dst, len);

  mach_write_to_2(src + FIL_PAGE_TYPE, header.m_original_type);

  ut_ad(dblwr_recover || memcmp(src + FIL_PAGE_LSN + 4,
                                src + (header.m_original_size + FIL_PAGE_DATA) -
                                    FIL_PAGE_END_LSN_OLD_CHKSUM + 4,
                                4) == 0);

  if (allocated) {
    ut_free(dst);
  }

  return (DB_SUCCESS);
}
