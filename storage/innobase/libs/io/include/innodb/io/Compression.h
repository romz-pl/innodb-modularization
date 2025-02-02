#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>

#include <string>

/** Compression algorithm. */
struct Compression {
  /** Algorithm types supported */
  enum Type {
    /* Note: During recovery we don't have the compression type
    because the .frm file has not been read yet. Therefore
    we write the recovered pages out without compression. */

    /** No compression */
    NONE = 0,

    /** Use ZLib */
    ZLIB = 1,

    /** Use LZ4 faster variant, usually lower compression. */
    LZ4 = 2
  };

  /** Compressed page meta-data */
  struct meta_t {
    /** Version number */
    uint8_t m_version;

    /** Algorithm type */
    Type m_algorithm;

    /** Original page type */
    uint16_t m_original_type;

    /** Original page size, before compression */
    uint16_t m_original_size;

    /** Size after compression */
    uint16_t m_compressed_size;
  };

  /** Default constructor */
  Compression() : m_type(NONE) {}

  /** Specific constructor
  @param[in]	type		Algorithm type */
  explicit Compression(Type type) : m_type(type) {
#ifdef UNIV_DEBUG
    switch (m_type) {
      case NONE:
      case ZLIB:
      case LZ4:

      default:
        ut_error;
    }
#endif /* UNIV_DEBUG */
  }

  /** Check the page header type field.
  @param[in]	page		Page contents
  @return true if it is a compressed page */
  static bool is_compressed_page(const byte *page)
      MY_ATTRIBUTE((warn_unused_result));

  /** Check wether the compression algorithm is supported.
  @param[in]      algorithm       Compression algorithm to check
  @param[out]     compression            The type that algorithm maps to
  @return DB_SUCCESS or error code */
  static dberr_t check(const char *algorithm, Compression *compression)
      MY_ATTRIBUTE((warn_unused_result));

  /** Validate the algorithm string.
  @param[in]      algorithm       Compression algorithm to check
  @return DB_SUCCESS or error code */
  static dberr_t validate(const char *algorithm)
      MY_ATTRIBUTE((warn_unused_result));

  /** Convert to a "string".
  @param[in]      type            The compression type
  @return the string representation */
  static const char *to_string(Type type) MY_ATTRIBUTE((warn_unused_result));

  /** Convert the meta data to a std::string.
  @param[in]      meta		Page Meta data
  @return the string representation */
  static std::string to_string(const meta_t &meta)
      MY_ATTRIBUTE((warn_unused_result));

  /** Deserizlise the page header compression meta-data
  @param[in]	page		Pointer to the page header
  @param[out]	control		Deserialised data */
  static void deserialize_header(const byte *page, meta_t *control);

  /** Check if the string is "empty" or "none".
  @param[in]      algorithm       Compression algorithm to check
  @return true if no algorithm requested */
  static bool is_none(const char *algorithm) MY_ATTRIBUTE((warn_unused_result));

  /** Decompress the page data contents. Page type must be
  FIL_PAGE_COMPRESSED, if not then the source contents are
  left unchanged and DB_SUCCESS is returned.
  @param[in]	dblwr_recover	true of double write recovery
                                  in progress
  @param[in,out]	src		Data read from disk, decompressed
                                  data will be copied to this page
  @param[in,out]	dst		Scratch area to use for decompression
  @param[in]	dst_len		Size of the scratch area in bytes
  @return DB_SUCCESS or error code */
  static dberr_t deserialize(bool dblwr_recover, byte *src, byte *dst,
                             ulint dst_len) MY_ATTRIBUTE((warn_unused_result));

  /** Compression type */
  Type m_type;
};
