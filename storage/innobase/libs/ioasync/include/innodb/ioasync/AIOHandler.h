#pragma once

#include <innodb/univ/univ.h>

#include <innodb/ioasync/Slot.h>
#include <innodb/io/os_file_io_complete.h>
#include <innodb/machine/data.h>
#include <innodb/page/page_type_t.h>
#include <innodb/page/type.h>

/** Generic AIO Handler methods. Currently handles IO post processing. */
class AIOHandler {
 public:
  /** Do any post processing after a read/write
  @return DB_SUCCESS or error code. */
  static dberr_t post_io_processing(Slot *slot);

  /** Decompress after a read and punch a hole in the file if
  it was a write */
  static dberr_t io_complete(const Slot *slot) {
    ut_a(slot->offset > 0);
    ut_a(slot->type.is_read() || !slot->skip_punch_hole);
    return (os_file_io_complete(slot->type, slot->file.m_file, slot->buf, NULL,
                                slot->original_len, slot->offset, slot->len));
  }

 private:
  /** Check whether the page was encrypted.
  @param[in]	slot		The slot that contains the IO request
  @return true if it was an encyrpted page */
  static bool is_encrypted_page(const Slot *slot) {
    return (Encryption::is_encrypted_page(slot->buf));
  }

  /** Check whether the page was compressed.
  @param[in]	slot		The slot that contains the IO request
  @return true if it was a compressed page */
  static bool is_compressed_page(const Slot *slot) {
    const byte *src = slot->buf;

    ulint page_type = mach_read_from_2(src + FIL_PAGE_TYPE);

    return (page_type == FIL_PAGE_COMPRESSED);
  }

  /** Get the compressed page size.
  @param[in]	slot		The slot that contains the IO request
  @return number of bytes to read for a successful decompress */
  static ulint compressed_page_size(const Slot *slot) {
    ut_ad(slot->type.is_read());
    ut_ad(is_compressed_page(slot));

    ulint size;
    const byte *src = slot->buf;

    size = mach_read_from_2(src + FIL_PAGE_COMPRESS_SIZE_V1);

    return (size + FIL_PAGE_DATA);
  }

  /** Check if the page contents can be decompressed.
  @param[in]	slot		The slot that contains the IO request
  @return true if the data read has all the compressed data */
  static bool can_decompress(const Slot *slot) {
    ut_ad(slot->type.is_read());
    ut_ad(is_compressed_page(slot));

    ulint version;
    const byte *src = slot->buf;

    version = mach_read_from_1(src + FIL_PAGE_VERSION);

    ut_a(version == 1);

    /* Includes the page header size too */
    ulint size = compressed_page_size(slot);

    return (size <= (slot->ptr - slot->buf) + (ulint)slot->n_bytes);
  }

  /** Check if we need to read some more data.
  @param[in]	slot		The slot that contains the IO request
  @param[in]	n_bytes		Total bytes read so far
  @return DB_SUCCESS or error code */
  static dberr_t check_read(Slot *slot, ulint n_bytes);
};

