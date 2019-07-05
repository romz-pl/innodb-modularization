#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>

/** UUID of server instance, it's needed for composing master key name */
constexpr ulint ENCRYPTION_SERVER_UUID_LEN = 36;


class IORequest;

/** Encryption algorithm. */
struct Encryption {
  /** Algorithm types supported */
  enum Type {

    /** No encryption */
    NONE = 0,

    /** Use AES */
    AES = 1,
  };

  /** Encryption information format version */
  enum Version {

    /** Version in 5.7.11 */
    ENCRYPTION_VERSION_1 = 0,

    /** Version in > 5.7.11 */
    ENCRYPTION_VERSION_2 = 1,

    /** Version in > 8.0.4 */
    ENCRYPTION_VERSION_3 = 2,
  };

  /** Default constructor */
  Encryption() : m_type(NONE) {}

  /** Specific constructor
  @param[in]	type		Algorithm type */
  explicit Encryption(Type type) : m_type(type) {
#ifdef UNIV_DEBUG
    switch (m_type) {
      case NONE:
      case AES:

      default:
        ut_error;
    }
#endif /* UNIV_DEBUG */
  }

  /** Copy constructor */
  Encryption(const Encryption &other)
      : m_type(other.m_type),
        m_key(other.m_key),
        m_klen(other.m_klen),
        m_iv(other.m_iv) {}

  Encryption &operator=(const Encryption &) = default;

  /** Check if page is encrypted page or not
  @param[in]	page	page which need to check
  @return true if it is an encrypted page */
  static bool is_encrypted_page(const byte *page)
      MY_ATTRIBUTE((warn_unused_result));

  /** Check if a log block is encrypted or not
  @param[in]	block	block which need to check
  @return true if it is an encrypted block */
  static bool is_encrypted_log(const byte *block)
      MY_ATTRIBUTE((warn_unused_result));

  /** Check the encryption option and set it
  @param[in]	option		encryption option
  @param[in,out]	encryption	The encryption type
  @return DB_SUCCESS or DB_UNSUPPORTED */
  dberr_t set_algorithm(const char *option, Encryption *type)
      MY_ATTRIBUTE((warn_unused_result));

  /** Validate the algorithm string.
  @param[in]	option		Encryption option
  @return DB_SUCCESS or error code */
  static dberr_t validate(const char *option)
      MY_ATTRIBUTE((warn_unused_result));

  /** Convert to a "string".
  @param[in]      type            The encryption type
  @return the string representation */
  static const char *to_string(Type type) MY_ATTRIBUTE((warn_unused_result));

  /** Check if the string is "empty" or "none".
  @param[in]      algorithm       Encryption algorithm to check
  @return true if no algorithm requested */
  static bool is_none(const char *algorithm) MY_ATTRIBUTE((warn_unused_result));

  /** Generate random encryption value for key and iv.
  @param[in,out]	value	Encryption value */
  static void random_value(byte *value);

  /** Create new master key for key rotation.
  @param[in,out]	master_key	master key */
  static void create_master_key(byte **master_key);

  /** Get master key by key id.
  @param[in]	master_key_id	master key id
  @param[in]	srv_uuid	uuid of server instance
  @param[in,out]	master_key	master key */
  static void get_master_key(ulint master_key_id, char *srv_uuid,
                             byte **master_key);

  /** Get current master key and key id.
  @param[in,out]	master_key_id	master key id
  @param[in,out]	master_key	master key */
  static void get_master_key(ulint *master_key_id, byte **master_key);

  /** Fill the encryption information.
  @param[in]	key		encryption key
  @param[in]	iv		encryption iv
  @param[in,out]	encrypt_info	encryption information
  @param[in]	is_boot		if it's for bootstrap
  @return true if success. */
  static bool fill_encryption_info(byte *key, byte *iv, byte *encrypt_info,
                                   bool is_boot);

  /** Get master key from encryption information
  @param[in]	encrypt_info	encryption information
  @param[in]	version		version of encryption information
  @param[in,out]	m_key_id	master key id
  @param[in,out]	srv_uuid	server uuid
  @param[in,out]	master_key	master key
  @return position after master key id or uuid, or the old position
  if can't get the master key. */
  static byte *get_master_key_from_info(byte *encrypt_info, Version version,
                                        uint32_t *m_key_id, char *srv_uuid,
                                        byte **master_key);

  /** Decoding the encryption info from the first page of a tablespace.
  @param[in,out]	key		key
  @param[in,out]	iv		iv
  @param[in]		encryption_info	encryption info
  @return true if success */
  static bool decode_encryption_info(byte *key, byte *iv,
                                     byte *encryption_info);

  /** Encrypt the redo log block.
  @param[in]	type		IORequest
  @param[in,out]	src_ptr		log block which need to encrypt
  @param[in,out]	dst_ptr		destination area
  @return true if success. */
  bool encrypt_log_block(const IORequest &type, byte *src_ptr, byte *dst_ptr);

  /** Encrypt the redo log data contents.
  @param[in]	type		IORequest
  @param[in,out]	src		page data which need to encrypt
  @param[in]	src_len		size of the source in bytes
  @param[in,out]	dst		destination area
  @param[in,out]	dst_len		size of the destination in bytes
  @return buffer data, dst_len will have the length of the data */
  byte *encrypt_log(const IORequest &type, byte *src, ulint src_len, byte *dst,
                    ulint *dst_len);

  /** Encrypt the page data contents. Page type can't be
  FIL_PAGE_ENCRYPTED, FIL_PAGE_COMPRESSED_AND_ENCRYPTED,
  FIL_PAGE_ENCRYPTED_RTREE.
  @param[in]	type		IORequest
  @param[in,out]	src		page data which need to encrypt
  @param[in]	src_len		size of the source in bytes
  @param[in,out]	dst		destination area
  @param[in,out]	dst_len		size of the destination in bytes
  @return buffer data, dst_len will have the length of the data */
  byte *encrypt(const IORequest &type, byte *src, ulint src_len, byte *dst,
                ulint *dst_len) MY_ATTRIBUTE((warn_unused_result));

  /** Decrypt the log block.
  @param[in]	type		IORequest
  @param[in,out]	src		data read from disk, decrypted data
                                  will be copied to this page
  @param[in,out]	dst		scratch area to use for decryption
  @return DB_SUCCESS or error code */
  dberr_t decrypt_log_block(const IORequest &type, byte *src, byte *dst);

  /** Decrypt the log data contents.
  @param[in]	type		IORequest
  @param[in,out]	src		data read from disk, decrypted data
                                  will be copied to this page
  @param[in]	src_len		source data length
  @param[in,out]	dst		scratch area to use for decryption
  @param[in]	dst_len		size of the scratch area in bytes
  @return DB_SUCCESS or error code */
  dberr_t decrypt_log(const IORequest &type, byte *src, ulint src_len,
                      byte *dst, ulint dst_len);

  /** Decrypt the page data contents. Page type must be
  FIL_PAGE_ENCRYPTED, FIL_PAGE_COMPRESSED_AND_ENCRYPTED,
  FIL_PAGE_ENCRYPTED_RTREE, if not then the source contents are
  left unchanged and DB_SUCCESS is returned.
  @param[in]	type		IORequest
  @param[in,out]	src		data read from disk, decrypt
                                  data will be copied to this page
  @param[in]	src_len		source data length
  @param[in,out]	dst		scratch area to use for decrypt
  @param[in]	dst_len		size of the scratch area in bytes
  @return DB_SUCCESS or error code */
  dberr_t decrypt(const IORequest &type, byte *src, ulint src_len, byte *dst,
                  ulint dst_len) MY_ATTRIBUTE((warn_unused_result));

  /** Check if keyring plugin loaded. */
  static bool check_keyring();

  /** Encrypt type */
  Type m_type;

  /** Encrypt key */
  byte *m_key;

  /** Encrypt key length*/
  ulint m_klen;

  /** Encrypt initial vector */
  byte *m_iv;

  /** Current master key id */
  static ulint s_master_key_id;

  /** Current uuid of server instance */
  static char s_uuid[ENCRYPTION_SERVER_UUID_LEN + 1];
};

/** Encryption key length */
static const ulint ENCRYPTION_KEY_LEN = 32;

/** Encryption magic bytes size */
static const ulint ENCRYPTION_MAGIC_SIZE = 3;

/** Encryption magic bytes for 5.7.11, it's for checking the encryption
information version. */
static const char ENCRYPTION_KEY_MAGIC_V1[] = "lCA";

/** Encryption magic bytes for 5.7.12+, it's for checking the encryption
information version. */
static const char ENCRYPTION_KEY_MAGIC_V2[] = "lCB";

/** Encryption magic bytes for 8.0.5+, it's for checking the encryption
information version. */
static const char ENCRYPTION_KEY_MAGIC_V3[] = "lCC";

/** Encryption master key prifix */
static const char ENCRYPTION_MASTER_KEY_PRIFIX[] = "INNODBKey";

/** Encryption master key prifix size */
static const ulint ENCRYPTION_MASTER_KEY_PRIFIX_LEN = 9;

/** Encryption master key prifix size */
static const ulint ENCRYPTION_MASTER_KEY_NAME_MAX_LEN = 100;


/** Encryption information total size: magic number + master_key_id +
key + iv + server_uuid + checksum */
static const ulint ENCRYPTION_INFO_SIZE =
    (ENCRYPTION_MAGIC_SIZE + sizeof(uint32) + (ENCRYPTION_KEY_LEN * 2) +
     ENCRYPTION_SERVER_UUID_LEN + sizeof(uint32));

/** Maximum size of Encryption information considering all formats v1, v2 & v3.
 */
static const ulint ENCRYPTION_INFO_MAX_SIZE =
    (ENCRYPTION_INFO_SIZE + sizeof(uint32));

/** Default master key for bootstrap */
static const char ENCRYPTION_DEFAULT_MASTER_KEY[] = "DefaultMasterKey";

/** Default master key id for bootstrap */
static const ulint ENCRYPTION_DEFAULT_MASTER_KEY_ID = 0;

/** (Un)Encryption Operation information size */
static const uint ENCRYPTION_OPERATION_INFO_SIZE = 1;

/** Encryption Progress information size */
static const uint ENCRYPTION_PROGRESS_INFO_SIZE = sizeof(uint);

/** Flag bit to indicate if Encryption/Decryption is in progress */
#define ENCRYPTION_IN_PROGRESS (1 << 0)
#define UNENCRYPTION_IN_PROGRESS (1 << 1)
