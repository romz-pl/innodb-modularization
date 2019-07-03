#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/Encryption.h>
#include <innodb/io/Compression.h>
#include <innodb/assert/assert.h>
#include "my_dbug.h"

/**
The IO Context that is passed down to the low level IO code */
class IORequest {
 public:
  /** Flags passed in the request, they can be ORred together. */
  enum {
    UNSET = 0,
    READ = 1,
    WRITE = 2,

    /** Double write buffer recovery. */
    DBLWR_RECOVER = 4,

    /** Enumerations below can be ORed to READ/WRITE above*/

    /** Data file */
    DATA_FILE = 8,

    /** Log file request*/
    LOG = 16,

    /** Disable partial read warnings */
    DISABLE_PARTIAL_IO_WARNINGS = 32,

    /** Do not to wake i/o-handler threads, but the caller will do
    the waking explicitly later, in this way the caller can post
    several requests in a batch; NOTE that the batch must not be
    so big that it exhausts the slots in AIO arrays! NOTE that
    a simulated batch may introduce hidden chances of deadlocks,
    because I/Os are not actually handled until all
    have been posted: use with great caution! */
    DO_NOT_WAKE = 64,

    /** Ignore failed reads of non-existent pages */
    IGNORE_MISSING = 128,

    /** Use punch hole if available, only makes sense if
    compression algorithm != NONE. Ignored if not set */
    PUNCH_HOLE = 256,

    /** Force raw read, do not try to compress/decompress.
    This can be used to force a read and write without any
    compression e.g., for redo log, merge sort temporary files
    and the truncate redo log. */
    NO_COMPRESSION = 512
  };

  /** Default constructor */
  IORequest()
      : m_block_size(UNIV_SECTOR_SIZE),
        m_type(READ),
        m_compression(),
        m_encryption() {
    /* No op */
  }

  /**
  @param[in]	type		Request type, can be a value that is
                                  ORed from the above enum */
  explicit IORequest(ulint type)
      : m_block_size(UNIV_SECTOR_SIZE),
        m_type(static_cast<uint16_t>(type)),
        m_compression(),
        m_encryption() {
    if (is_log()) {
      disable_compression();
    }

    if (!is_punch_hole_supported()) {
      clear_punch_hole();
    }
  }

  /** @return true if ignore missing flag is set */
  static bool ignore_missing(ulint type) MY_ATTRIBUTE((warn_unused_result)) {
    return ((type & IGNORE_MISSING) == IGNORE_MISSING);
  }

  /** @return true if it is a read request */
  bool is_read() const MY_ATTRIBUTE((warn_unused_result)) {
    return ((m_type & READ) == READ);
  }

  /** @return true if it is a write request */
  bool is_write() const MY_ATTRIBUTE((warn_unused_result)) {
    return ((m_type & WRITE) == WRITE);
  }

  /** @return true if it is a redo log write */
  bool is_log() const MY_ATTRIBUTE((warn_unused_result)) {
    return ((m_type & LOG) == LOG);
  }

  /** @return true if the simulated AIO thread should be woken up */
  bool is_wake() const MY_ATTRIBUTE((warn_unused_result)) {
    return ((m_type & DO_NOT_WAKE) == 0);
  }

  /** @return true if partial read warning disabled */
  bool is_partial_io_warning_disabled() const
      MY_ATTRIBUTE((warn_unused_result)) {
    return ((m_type & DISABLE_PARTIAL_IO_WARNINGS) ==
            DISABLE_PARTIAL_IO_WARNINGS);
  }

  /** Disable partial read warnings */
  void disable_partial_io_warnings() { m_type |= DISABLE_PARTIAL_IO_WARNINGS; }

  /** @return true if missing files should be ignored */
  bool ignore_missing() const MY_ATTRIBUTE((warn_unused_result)) {
    return (ignore_missing(m_type));
  }

  /** @return true if punch hole should be used */
  bool punch_hole() const MY_ATTRIBUTE((warn_unused_result)) {
    return ((m_type & PUNCH_HOLE) == PUNCH_HOLE);
  }

  /** @return true if the read should be validated */
  bool validate() const MY_ATTRIBUTE((warn_unused_result)) {
    ut_a(is_read() ^ is_write());

    return (!is_read() || !punch_hole());
  }

  /** Set the punch hole flag */
  void set_punch_hole() {
    if (is_punch_hole_supported()) {
      m_type |= PUNCH_HOLE;
    }
  }

  /** Clear the do not wake flag */
  void clear_do_not_wake() { m_type &= ~DO_NOT_WAKE; }

  /** Clear the punch hole flag */
  void clear_punch_hole() { m_type &= ~PUNCH_HOLE; }

  /** @return the block size to use for IO */
  ulint block_size() const MY_ATTRIBUTE((warn_unused_result)) {
    return (m_block_size);
  }

  /** Set the block size for IO
  @param[in] block_size		Block size to set */
  void block_size(ulint block_size) {
    m_block_size = static_cast<uint32_t>(block_size);
  }

  /** Clear all compression related flags */
  void clear_compressed() {
    clear_punch_hole();

    m_compression.m_type = Compression::NONE;
  }

  /** Compare two requests
  @return true if the are equal */
  bool operator==(const IORequest &rhs) const { return (m_type == rhs.m_type); }

  /** Set compression algorithm
  @param[in]	type	The compression algorithm to use */
  void compression_algorithm(Compression::Type type) {
    if (type == Compression::NONE) {
      return;
    }

    set_punch_hole();

    m_compression.m_type = type;
  }

  /** Get the compression algorithm.
  @return the compression algorithm */
  Compression compression_algorithm() const MY_ATTRIBUTE((warn_unused_result)) {
    return (m_compression);
  }

  /** @return true if the page should be compressed */
  bool is_compressed() const MY_ATTRIBUTE((warn_unused_result)) {
    return (compression_algorithm().m_type != Compression::NONE);
  }

  /** @return true if the page read should not be transformed. */
  bool is_compression_enabled() const MY_ATTRIBUTE((warn_unused_result)) {
    return ((m_type & NO_COMPRESSION) == 0);
  }

  /** Disable transformations. */
  void disable_compression() { m_type |= NO_COMPRESSION; }

  /** Set encryption algorithm
  @param[in] type		The encryption algorithm to use */
  void encryption_algorithm(Encryption::Type type) {
    if (type == Encryption::NONE) {
      return;
    }

    m_encryption.m_type = type;
  }

  /** Set encryption key and iv
  @param[in] key		The encryption key to use
  @param[in] key_len	length of the encryption key
  @param[in] iv		The encryption iv to use */
  void encryption_key(byte *key, ulint key_len, byte *iv) {
    m_encryption.m_key = key;
    m_encryption.m_klen = key_len;
    m_encryption.m_iv = iv;
  }

  /** Get the encryption algorithm.
  @return the encryption algorithm */
  Encryption encryption_algorithm() const MY_ATTRIBUTE((warn_unused_result)) {
    return (m_encryption);
  }

  /** @return true if the page should be encrypted. */
  bool is_encrypted() const MY_ATTRIBUTE((warn_unused_result)) {
    return (m_encryption.m_type != Encryption::NONE);
  }

  /** Clear all encryption related flags */
  void clear_encrypted() {
    m_encryption.m_key = NULL;
    m_encryption.m_klen = 0;
    m_encryption.m_iv = NULL;
    m_encryption.m_type = Encryption::NONE;
  }

  /** Note that the IO is for double write recovery. */
  void dblwr_recover() { m_type |= DBLWR_RECOVER; }

  /** @return true if the request is from the dblwr recovery */
  bool is_dblwr_recover() const MY_ATTRIBUTE((warn_unused_result)) {
    return ((m_type & DBLWR_RECOVER) == DBLWR_RECOVER);
  }

  /** @return true if punch hole is supported */
  static bool is_punch_hole_supported() {
    /* In this debugging mode, we act as if punch hole is supported,
    and then skip any calls to actually punch a hole here.
    In this way, Transparent Page Compression is still being tested. */
    DBUG_EXECUTE_IF("ignore_punch_hole", return (true););

#if defined(HAVE_FALLOC_PUNCH_HOLE_AND_KEEP_SIZE) || defined(_WIN32)
    return (true);
#else
    return (false);
#endif /* HAVE_FALLOC_PUNCH_HOLE_AND_KEEP_SIZE || _WIN32 */
  }

 private:
  /* File system best block size */
  uint32_t m_block_size;

  /** Request type bit flags */
  uint16_t m_type;

  /** Compression algorithm */
  Compression m_compression;

  /** Encryption algorithm */
  Encryption m_encryption;
};

