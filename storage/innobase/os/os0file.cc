/***********************************************************************

Copyright (c) 1995, 2019, Oracle and/or its affiliates. All Rights Reserved.
Copyright (c) 2009, Percona Inc.

Portions of this file contain modifications contributed and copyrighted
by Percona Inc.. Those modifications are
gratefully acknowledged and are described briefly in the InnoDB
documentation. The contributions by Percona Inc. are incorporated with
their permission, and subject to the conditions contained in the file
COPYING.Percona.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License, version 2.0,
as published by the Free Software Foundation.

This program is also distributed with certain software (including
but not limited to OpenSSL) that is licensed under separate terms,
as designated in a particular file or component or in included license
documentation.  The authors of MySQL hereby grant you an additional
permission to link the program and your derivative works with the
separately licensed software that they have included with MySQL.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License, version 2.0, for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

***********************************************************************/

/** @file os/os0file.cc
 The interface to the operating system file i/o primitives

 Created 10/21/1995 Heikki Tuuri
 *******************************************************/

#include <innodb/time/ut_time.h>
#include <innodb/logger/info.h>
#include <innodb/sync_event/os_event_wait.h>
#include <innodb/sync_event/os_event_reset.h>
#include <innodb/sync_event/os_event_is_set.h>
#include <innodb/ioasync/Slot.h>
#include <innodb/ioasync/AIO.h>
#include <innodb/ioasync/os_aio_n_segments.h>
#include <innodb/io/os_bytes_read_since_printout.h>
#include <innodb/io/os_file_read_page.h>
#include <innodb/io/os_n_fsyncs.h>
#include <innodb/ioasync/os_last_printout.h>
#include <innodb/ioasync/AIOHandler.h>
#include <innodb/ioasync/LinuxAIOHandler.h>
#include <innodb/io/srv_use_native_aio.h>
#include <innodb/io/srv_is_being_started.h>
#include <innodb/io/OPERATING_SYSTEM_ERROR_MSG.h>
#include <innodb/ioasync/SimulatedAIOHandler.h>
#include <innodb/ioasync/os_aio_simulated_handler.h>
#include <innodb/ioasync/os_aio_linux_handler.h>
#include <innodb/ioasync/os_aio_recommend_sleep_for_read_threads.h>

#include "os0file.h"
#include "my_compiler.h"
#include "my_dbug.h"
#include "my_inttypes.h"
#include "my_io.h"
#include "sql_const.h"
#include "srv0start.h"

#include <lz4.h>
#include <zlib.h>



#ifdef UNIV_HOTBACKUP
#include "fil0fil.h"

/** MEB routine to get the master key. MEB will extract
the key from the keyring encrypted file stored in backup.
@param[in]	key_id		the id of the master key
@param[in]	key_type	master key type
@param[out]	key		the master key being returned
@param[out]	key_length	the length of the returned key
@retval	0 if the key is being returned, 1 otherwise. */
extern int meb_key_fetch(const char *key_id, char **key_type,
                         const char *user_id, void **key, size_t *key_length);
#endif /* UNIV_HOTBACKUP */


#ifdef _WIN32
#include <errno.h>
#include <mbstring.h>
#include <sys/stat.h>
#include <tchar.h>
#include <codecvt>
#endif /* _WIN32 */

#ifdef __linux__
#include <sys/sendfile.h>
#endif /* __linux__ */

#ifdef LINUX_NATIVE_AIO
#ifndef UNIV_HOTBACKUP
#include <libaio.h>
#else /* !UNIV_HOTBACKUP */
#undef LINUX_NATIVE_AIO
#endif /* !UNIV_HOTBACKUP */
#endif /* LINUX_NATIVE_AIO */

#ifdef HAVE_FALLOC_PUNCH_HOLE_AND_KEEP_SIZE
#include <fcntl.h>
#include <linux/falloc.h>
#endif /* HAVE_FALLOC_PUNCH_HOLE_AND_KEEP_SIZE */

#include <errno.h>
#include <lz4.h>
#include "my_aes.h"
#include "my_rnd.h"
#include "mysql/service_mysql_keyring.h"
#include "mysqld.h"

#include <sys/types.h>
#include <zlib.h>
#include <ctime>
#include <functional>
#include <new>
#include <vector>

#include <innodb/io/Block.h>
#include <innodb/io/Blocks.h>
#include <innodb/io/block_cache.h>
#include <innodb/io/os_has_said_disk_full.h>
#include <innodb/io/macros.h>
#include <innodb/ioasync/os_aio_segment_wait_events.h>
#include <innodb/ioasync/buf_page_cleaner_is_active.h>

#ifdef UNIV_HOTBACKUP
#include <data0type.h>
#endif /* UNIV_HOTBACKUP */




#ifdef UNIV_ENABLE_UNIT_TEST_GET_PARENT_DIR

/* Test the function os_file_get_parent_dir. */
void test_os_file_get_parent_dir(const char *child_dir,
                                 const char *expected_dir) {
  char *child = mem_strdup(child_dir);
  char *expected = expected_dir == NULL ? NULL : mem_strdup(expected_dir);

  /* os_file_get_parent_dir() assumes that separators are
  converted to OS_PATH_SEPARATOR. */
  Fil_path::normalize(child);
  Fil_path::normalize(expected);

  char *parent = os_file_get_parent_dir(child);

  bool unexpected =
      (expected == NULL ? (parent != NULL) : (0 != strcmp(parent, expected)));
  if (unexpected) {
    ib::fatal(ER_IB_MSG_752)
        << "os_file_get_parent_dir('" << child << "') returned '" << parent
        << "', instead of '" << expected << "'.";
  }
  ut_free(parent);
  ut_free(child);
  ut_free(expected);
}

/* Test the function os_file_get_parent_dir. */
void unit_test_os_file_get_parent_dir() {
  test_os_file_get_parent_dir("/usr/lib/a", "/usr/lib");
  test_os_file_get_parent_dir("/usr/", NULL);
  test_os_file_get_parent_dir("//usr//", NULL);
  test_os_file_get_parent_dir("usr", NULL);
  test_os_file_get_parent_dir("usr//", NULL);
  test_os_file_get_parent_dir("/", NULL);
  test_os_file_get_parent_dir("//", NULL);
  test_os_file_get_parent_dir(".", NULL);
  test_os_file_get_parent_dir("..", NULL);
#ifdef _WIN32
  test_os_file_get_parent_dir("D:", NULL);
  test_os_file_get_parent_dir("D:/", NULL);
  test_os_file_get_parent_dir("D:\\", NULL);
  test_os_file_get_parent_dir("D:/data", NULL);
  test_os_file_get_parent_dir("D:/data/", NULL);
  test_os_file_get_parent_dir("D:\\data\\", NULL);
  test_os_file_get_parent_dir("D:///data/////", NULL);
  test_os_file_get_parent_dir("D:\\\\\\data\\\\\\\\", NULL);
  test_os_file_get_parent_dir("D:/data//a", "D:/data");
  test_os_file_get_parent_dir("D:\\data\\\\a", "D:\\data");
  test_os_file_get_parent_dir("D:///data//a///b/", "D:///data//a");
  test_os_file_get_parent_dir("D:\\\\\\data\\\\a\\\\\\b\\",
                              "D:\\\\\\data\\\\a");
#endif /* _WIN32 */
}
#endif /* UNIV_ENABLE_UNIT_TEST_GET_PARENT_DIR */


/** Decoding the encryption info from the first page of a tablespace.
@param[in,out]	key		key
@param[in,out]	iv		iv
@param[in]	encryption_info	encryption info
@return true if success */
bool Encryption::decode_encryption_info(byte *key, byte *iv,
                                        byte *encryption_info) {
  byte *ptr;
  byte *master_key = nullptr;
  uint32 m_key_id;
  byte key_info[ENCRYPTION_KEY_LEN * 2];
  ulint crc1;
  ulint crc2;
  char srv_uuid[ENCRYPTION_SERVER_UUID_LEN + 1];
  Version version;
#ifdef UNIV_ENCRYPT_DEBUG
  const byte *data;
  ulint i;
#endif

  ptr = encryption_info;

  /* For compatibility with 5.7.11, we need to handle the
  encryption information which created in this old version. */
  if (memcmp(ptr, ENCRYPTION_KEY_MAGIC_V1, ENCRYPTION_MAGIC_SIZE) == 0) {
    version = ENCRYPTION_VERSION_1;
  } else if (memcmp(ptr, ENCRYPTION_KEY_MAGIC_V2, ENCRYPTION_MAGIC_SIZE) == 0) {
    version = ENCRYPTION_VERSION_2;
  } else if (memcmp(ptr, ENCRYPTION_KEY_MAGIC_V3, ENCRYPTION_MAGIC_SIZE) == 0) {
    version = ENCRYPTION_VERSION_3;
  } else {
    /* We don't report an error during recovery, since the
    encryption info maybe hasn't writen into datafile when
    the table is newly created. */
    if (recv_recovery_is_on()) {
      return (true);
    }

    ib::error(ER_IB_MSG_837) << "Failed to decrypt encryption information,"
                             << " found unexpected version of it!";
    return (false);
  }

  ptr += ENCRYPTION_MAGIC_SIZE;

  /* Get master key by key id. */
  ptr =
      get_master_key_from_info(ptr, version, &m_key_id, srv_uuid, &master_key);

  /* If can't find the master key, return failure. */
  if (master_key == nullptr) {
    return (false);
  }

#ifdef UNIV_ENCRYPT_DEBUG
  {
    std::ostringstream msg;

    ut_print_buf_hex(msg, master_key, ENCRYPTION_KEY_LEN);

    ib::info(ER_IB_MSG_838)
        << "Key ID: " << key_id << " hex: {" << msg.str() << "}";
  }
#endif /* UNIV_ENCRYPT_DEBUG */

  /* Decrypt tablespace key and iv. */
  auto len = my_aes_decrypt(ptr, sizeof(key_info), key_info, master_key,
                            ENCRYPTION_KEY_LEN, my_aes_256_ecb, nullptr, false);

  if (m_key_id == 0) {
    ut_free(master_key);
  } else {
    my_free(master_key);
  }

  /* If decryption failed, return error. */
  if (len == MY_AES_BAD_DATA) {
    return (false);
  }

  /* Check checksum bytes. */
  ptr += sizeof(key_info);

  crc1 = mach_read_from_4(ptr);
  crc2 = ut_crc32(key_info, sizeof(key_info));

  if (crc1 != crc2) {
    ib::error(ER_IB_MSG_839)
        << "Failed to decrypt encryption information,"
        << " please check whether key file has been changed!";

    return (false);
  }

  /* Get tablespace key */
  memcpy(key, key_info, ENCRYPTION_KEY_LEN);

  /* Get tablespace iv */
  memcpy(iv, key_info + ENCRYPTION_KEY_LEN, ENCRYPTION_KEY_LEN);

#ifdef UNIV_ENCRYPT_DEBUG
  {
    std::ostringstream msg;

    ut_print_buf_hex(msg, key, ENCRYPTION_KEY_LEN);

    ib::info(ER_IB_MSG_840) << "Key: {" << msg.str() << "}";
  }
  {
    std::ostringstream msg;

    ut_print_buf_hex(msg, iv, ENCRYPTION_KEY_LEN);
    ib::info(ER_IB_MSG_841) << "IV: {" << msg.str() << "}";
  }
#endif /* UNIV_ENCRYPT_DEBUG */

  if (s_master_key_id < m_key_id) {
    s_master_key_id = m_key_id;
    memcpy(s_uuid, srv_uuid, sizeof(s_uuid) - 1);
  }

  return (true);
}




/** Check if redo log block is encrypted block or not
@param[in]	block	log block to check
@return true if it is an encrypted block */
bool Encryption::is_encrypted_log(const byte *block) {
  return (log_block_get_encrypt_bit(block));
}

/** Encrypt the redo log block.
@param[in]	type		IORequest
@param[in]	src_ptr		log block which need to encrypt
@param[in,out]	dst_ptr		destination area
@return true if success. */
bool Encryption::encrypt_log_block(const IORequest &type, byte *src_ptr,
                                   byte *dst_ptr) {
  ulint len = 0;
  ulint data_len;
  ulint main_len;
  ulint remain_len;
  byte remain_buf[MY_AES_BLOCK_SIZE * 2];

#ifdef UNIV_ENCRYPT_DEBUG
  {
    std::ostringstream msg;

    ut_print_buf_hex(msg, src_ptr, OS_FILE_LOG_BLOCK_SIZE);

    ib::info(ER_IB_MSG_842)
        << "Encrypting block: " << log_block_get_hdr_no(src_ptr) << "{"
        << msg.str() << "}";
  }
#endif /* UNIV_ENCRYPT_DEBUG */

  /* This is data size which need to encrypt. */
  data_len = OS_FILE_LOG_BLOCK_SIZE - LOG_BLOCK_HDR_SIZE;
  main_len = (data_len / MY_AES_BLOCK_SIZE) * MY_AES_BLOCK_SIZE;
  remain_len = data_len - main_len;

  /* Encrypt the block. */
  /* Copy the header as is. */
  memmove(dst_ptr, src_ptr, LOG_BLOCK_HDR_SIZE);
  ut_ad(memcmp(src_ptr, dst_ptr, LOG_BLOCK_HDR_SIZE) == 0);

  switch (m_type) {
    case Encryption::NONE:
      ut_error;

    case Encryption::AES: {
      ut_ad(m_klen == ENCRYPTION_KEY_LEN);

      auto elen = my_aes_encrypt(
          src_ptr + LOG_BLOCK_HDR_SIZE, static_cast<uint32>(main_len),
          dst_ptr + LOG_BLOCK_HDR_SIZE,
          reinterpret_cast<unsigned char *>(m_key), static_cast<uint32>(m_klen),
          my_aes_256_cbc, reinterpret_cast<unsigned char *>(m_iv), false);

      if (elen == MY_AES_BAD_DATA) {
        return (false);
      }

      len = static_cast<ulint>(elen);
      ut_ad(len == main_len);

      /* Copy remain bytes. */
      memcpy(dst_ptr + LOG_BLOCK_HDR_SIZE + len,
             src_ptr + LOG_BLOCK_HDR_SIZE + len,
             OS_FILE_LOG_BLOCK_SIZE - LOG_BLOCK_HDR_SIZE - len);

      /* Encrypt the remain bytes. Since my_aes_encrypt
      request the content which need to encrypt is
      multiple of MY_AES_BLOCK_SIZE, but the block
      content is possiblly not, so, we need to handle
      the tail bytes first. */
      if (remain_len != 0) {
        remain_len = MY_AES_BLOCK_SIZE * 2;

        elen =
            my_aes_encrypt(dst_ptr + LOG_BLOCK_HDR_SIZE + data_len - remain_len,
                           static_cast<uint32>(remain_len), remain_buf,
                           reinterpret_cast<unsigned char *>(m_key),
                           static_cast<uint32>(m_klen), my_aes_256_cbc,
                           reinterpret_cast<unsigned char *>(m_iv), false);

        if (elen == MY_AES_BAD_DATA) {
          return (false);
        }

        memcpy(dst_ptr + LOG_BLOCK_HDR_SIZE + data_len - remain_len, remain_buf,
               remain_len);
      }

      break;
    }

    default:
      ut_error;
  }

#ifdef UNIV_ENCRYPT_DEBUG
  fprintf(stderr, "Encrypted block %lu.\n", log_block_get_hdr_no(dst_ptr));
  ut_print_buf_hex(stderr, dst_ptr, OS_FILE_LOG_BLOCK_SIZE);
  fprintf(stderr, "\n");

  byte *check_buf =
      static_cast<byte *>(ut_malloc_nokey(OS_FILE_LOG_BLOCK_SIZE));
  byte *buf2 = static_cast<byte *>(ut_malloc_nokey(OS_FILE_LOG_BLOCK_SIZE));

  memcpy(check_buf, dst_ptr, OS_FILE_LOG_BLOCK_SIZE);
  dberr_t err = decrypt_log(type, check_buf, OS_FILE_LOG_BLOCK_SIZE, buf2,
                            OS_FILE_LOG_BLOCK_SIZE);
  log_block_set_encrypt_bit(check_buf, true);
  if (err != DB_SUCCESS ||
      memcmp(src_ptr, check_buf, OS_FILE_LOG_BLOCK_SIZE) != 0) {
    ut_print_buf_hex(stderr, src_ptr, OS_FILE_LOG_BLOCK_SIZE);
    ut_print_buf_hex(stderr, check_buf, OS_FILE_LOG_BLOCK_SIZE);
    ut_ad(0);
  }
  ut_free(buf2);
  ut_free(check_buf);
#endif /* UNIV_ENCRYPT_DEBUG */

  /* Set the encrypted flag. */
  log_block_set_encrypt_bit(dst_ptr, true);

  return (true);
}

/** Encrypt the redo log data contents.
@param[in]	type		IORequest
@param[in]	src		page data which need to encrypt
@param[in]	src_len		Size of the source in bytes
@param[in,out]	dst		destination area
@param[in,out]	dst_len		Size of the destination in bytes
@return buffer data, dst_len will have the length of the data */
byte *Encryption::encrypt_log(const IORequest &type, byte *src, ulint src_len,
                              byte *dst, ulint *dst_len) {
  byte *src_ptr = src;
  byte *dst_ptr = dst;

  ut_ad(type.is_log());
  ut_ad(src_len % OS_FILE_LOG_BLOCK_SIZE == 0);
  ut_ad(m_type != Encryption::NONE);

  /* Encrypt the log blocks one by one. */
  while (src_ptr != src + src_len) {
    if (!encrypt_log_block(type, src_ptr, dst_ptr)) {
      *dst_len = src_len;
      ib::error(ER_IB_MSG_843) << " Can't encrypt data of"
                               << " redo log";
      return (src);
    }

    src_ptr += OS_FILE_LOG_BLOCK_SIZE;
    dst_ptr += OS_FILE_LOG_BLOCK_SIZE;
  }

#ifdef UNIV_ENCRYPT_DEBUG
  byte *check_buf = static_cast<byte *>(ut_malloc_nokey(src_len));
  byte *buf2 = static_cast<byte *>(ut_malloc_nokey(src_len));

  memcpy(check_buf, dst, src_len);

  dberr_t err = decrypt_log(type, check_buf, src_len, buf2, src_len);
  if (err != DB_SUCCESS || memcmp(src, check_buf, src_len) != 0) {
    ut_print_buf_hex(stderr, src, src_len);
    ut_print_buf_hex(stderr, check_buf, src_len);
    ut_ad(0);
  }
  ut_free(buf2);
  ut_free(check_buf);
#endif /* UNIV_ENCRYPT_DEBUG */

  return (dst);
}

/** Encrypt the page data contents. Page type can't be
FIL_PAGE_ENCRYPTED, FIL_PAGE_COMPRESSED_AND_ENCRYPTED,
FIL_PAGE_ENCRYPTED_RTREE.
@param[in]	type		IORequest
@param[in]	src		page data which need to encrypt
@param[in]	src_len		Size of the source in bytes
@param[in,out]	dst		destination area
@param[in,out]	dst_len		Size of the destination in bytes
@return buffer data, dst_len will have the length of the data */
byte *Encryption::encrypt(const IORequest &type, byte *src, ulint src_len,
                          byte *dst, ulint *dst_len) {
  ulint len = 0;
  ulint page_type = mach_read_from_2(src + FIL_PAGE_TYPE);
  ulint data_len;
  ulint main_len;
  ulint remain_len;
  byte remain_buf[MY_AES_BLOCK_SIZE * 2];

  /* For encrypting redo log, take another way. */
  ut_ad(!type.is_log());

#ifdef UNIV_ENCRYPT_DEBUG
  ulint space_id = mach_read_from_4(src + FIL_PAGE_ARCH_LOG_NO_OR_SPACE_ID);
  ulint page_no = mach_read_from_4(src + FIL_PAGE_OFFSET);

  fprintf(stderr, "Encrypting page:%lu.%lu len:%lu\n", space_id, page_no,
          src_len);
  ut_print_buf(stderr, m_key, 32);
  ut_print_buf(stderr, m_iv, 32);
#endif /* UNIV_ENCRYPT_DEBUG */

  /* Shouldn't encrypte an already encrypted page. */
  ut_ad(page_type != FIL_PAGE_ENCRYPTED &&
        page_type != FIL_PAGE_COMPRESSED_AND_ENCRYPTED &&
        page_type != FIL_PAGE_ENCRYPTED_RTREE);

  ut_ad(m_type != Encryption::NONE);

  /* This is data size which need to encrypt. */
  data_len = src_len - FIL_PAGE_DATA;
  main_len = (data_len / MY_AES_BLOCK_SIZE) * MY_AES_BLOCK_SIZE;
  remain_len = data_len - main_len;

  /* Only encrypt the data + trailer, leave the header alone */

  switch (m_type) {
    case Encryption::NONE:
      ut_error;

    case Encryption::AES: {
      lint elen;

      ut_ad(m_klen == ENCRYPTION_KEY_LEN);

      elen = my_aes_encrypt(src + FIL_PAGE_DATA, static_cast<uint32>(main_len),
                            dst + FIL_PAGE_DATA,
                            reinterpret_cast<unsigned char *>(m_key),
                            static_cast<uint32>(m_klen), my_aes_256_cbc,
                            reinterpret_cast<unsigned char *>(m_iv), false);

      if (elen == MY_AES_BAD_DATA) {
        ulint page_no = mach_read_from_4(src + FIL_PAGE_OFFSET);
        ulint space_id =
            mach_read_from_4(src + FIL_PAGE_ARCH_LOG_NO_OR_SPACE_ID);
        *dst_len = src_len;
        ib::error(ER_IB_MSG_844)
            << " Can't encrypt data of page,"
            << " page no:" << page_no << " space id:" << space_id;
        return (src);
      }

      len = static_cast<ulint>(elen);
      ut_ad(len == main_len);

      /* Copy remain bytes and page tailer. */
      memcpy(dst + FIL_PAGE_DATA + len, src + FIL_PAGE_DATA + len,
             src_len - FIL_PAGE_DATA - len);

      /* Encrypt the remain bytes. */
      if (remain_len != 0) {
        remain_len = MY_AES_BLOCK_SIZE * 2;

        elen = my_aes_encrypt(dst + FIL_PAGE_DATA + data_len - remain_len,
                              static_cast<uint32>(remain_len), remain_buf,
                              reinterpret_cast<unsigned char *>(m_key),
                              static_cast<uint32>(m_klen), my_aes_256_cbc,
                              reinterpret_cast<unsigned char *>(m_iv), false);

        if (elen == MY_AES_BAD_DATA) {
          ulint page_no = mach_read_from_4(src + FIL_PAGE_OFFSET);
          ulint space_id =
              mach_read_from_4(src + FIL_PAGE_ARCH_LOG_NO_OR_SPACE_ID);

          ib::error(ER_IB_MSG_845)
              << " Can't encrypt data of page,"
              << " page no:" << page_no << " space id:" << space_id;
          *dst_len = src_len;
          return (src);
        }

        memcpy(dst + FIL_PAGE_DATA + data_len - remain_len, remain_buf,
               remain_len);
      }

      break;
    }

    default:
      ut_error;
  }

  /* Copy the header as is. */
  memmove(dst, src, FIL_PAGE_DATA);
  ut_ad(memcmp(src, dst, FIL_PAGE_DATA) == 0);

  /* Add encryption control information. Required for decrypting. */
  if (page_type == FIL_PAGE_COMPRESSED) {
    /* If the page is compressed, we don't need to save the
    original type, since it is done in compression already. */
    mach_write_to_2(dst + FIL_PAGE_TYPE, FIL_PAGE_COMPRESSED_AND_ENCRYPTED);
    ut_ad(memcmp(src + FIL_PAGE_TYPE + 2, dst + FIL_PAGE_TYPE + 2,
                 FIL_PAGE_DATA - FIL_PAGE_TYPE - 2) == 0);
  } else if (page_type == FIL_PAGE_RTREE) {
    /* If the page is R-tree page, we need to save original
    type. */
    mach_write_to_2(dst + FIL_PAGE_TYPE, FIL_PAGE_ENCRYPTED_RTREE);
  } else {
    mach_write_to_2(dst + FIL_PAGE_TYPE, FIL_PAGE_ENCRYPTED);
    mach_write_to_2(dst + FIL_PAGE_ORIGINAL_TYPE_V1, page_type);
  }

#ifdef UNIV_ENCRYPT_DEBUG
  byte *check_buf = static_cast<byte *>(ut_malloc_nokey(src_len));
  byte *buf2 = static_cast<byte *>(ut_malloc_nokey(src_len));

  memcpy(check_buf, dst, src_len);

  dberr_t err = decrypt(type, check_buf, src_len, buf2, src_len);
  if (err != DB_SUCCESS ||
      memcmp(src + FIL_PAGE_DATA, check_buf + FIL_PAGE_DATA,
             src_len - FIL_PAGE_DATA) != 0) {
    ut_print_buf(stderr, src, src_len);
    ut_print_buf(stderr, check_buf, src_len);
    ut_ad(0);
  }
  ut_free(buf2);
  ut_free(check_buf);

  fprintf(stderr, "Encrypted page:%lu.%lu\n", space_id, page_no);
#endif /* UNIV_ENCRYPT_DEBUG */

  *dst_len = src_len;

  return (dst);
}

/** Decrypt the log block.
@param[in]	type		IORequest
@param[in,out]	src		Data read from disk, decrypted data will be
                                copied to this page
@param[in,out]	dst		Scratch area to use for decryption
@return DB_SUCCESS or error code */
dberr_t Encryption::decrypt_log_block(const IORequest &type, byte *src,
                                      byte *dst) {
  ulint data_len;
  ulint main_len;
  ulint remain_len;
  byte remain_buf[MY_AES_BLOCK_SIZE * 2];
  byte *ptr = src;

  /* This is data size which need to encrypt. */
  data_len = OS_FILE_LOG_BLOCK_SIZE - LOG_BLOCK_HDR_SIZE;
  main_len = (data_len / MY_AES_BLOCK_SIZE) * MY_AES_BLOCK_SIZE;
  remain_len = data_len - main_len;

  ptr += LOG_BLOCK_HDR_SIZE;
  switch (m_type) {
    case Encryption::AES: {
      lint elen;

      /* First decrypt the last 2 blocks data of data, since
      data is no block aligned. */
      if (remain_len != 0) {
        ut_ad(m_klen == ENCRYPTION_KEY_LEN);

        remain_len = MY_AES_BLOCK_SIZE * 2;

        /* Copy the last 2 blocks. */
        memcpy(remain_buf, ptr + data_len - remain_len, remain_len);

        elen = my_aes_decrypt(remain_buf, static_cast<uint32>(remain_len),
                              dst + data_len - remain_len,
                              reinterpret_cast<unsigned char *>(m_key),
                              static_cast<uint32>(m_klen), my_aes_256_cbc,
                              reinterpret_cast<unsigned char *>(m_iv), false);
        if (elen == MY_AES_BAD_DATA) {
          return (DB_IO_DECRYPT_FAIL);
        }

        /* Copy the other data bytes to temp area. */
        memcpy(dst, ptr, data_len - remain_len);
      } else {
        ut_ad(data_len == main_len);

        /* Copy the data bytes to temp area. */
        memcpy(dst, ptr, data_len);
      }

      /* Then decrypt the main data */
      elen = my_aes_decrypt(dst, static_cast<uint32>(main_len), ptr,
                            reinterpret_cast<unsigned char *>(m_key),
                            static_cast<uint32>(m_klen), my_aes_256_cbc,
                            reinterpret_cast<unsigned char *>(m_iv), false);
      if (elen == MY_AES_BAD_DATA) {
        return (DB_IO_DECRYPT_FAIL);
      }

      ut_ad(static_cast<ulint>(elen) == main_len);

      /* Copy the remain bytes. */
      memcpy(ptr + main_len, dst + main_len, data_len - main_len);

      break;
    }

    default:
      ib::error(ER_IB_MSG_846) << "Encryption algorithm support missing: "
                               << Encryption::to_string(m_type);
      return (DB_UNSUPPORTED);
  }

  ptr -= LOG_BLOCK_HDR_SIZE;

#ifdef UNIV_ENCRYPT_DEBUG
  fprintf(stderr, "Decrypted block %lu.\n", log_block_get_hdr_no(ptr));
  ut_print_buf_hex(stderr, ptr, OS_FILE_LOG_BLOCK_SIZE);
  fprintf(stderr, "\n");
#endif

  /* Reset the encrypted flag. */
  log_block_set_encrypt_bit(ptr, false);

  return (DB_SUCCESS);
}

/** Decrypt the log data contents.
@param[in]	type		IORequest
@param[in,out]	src		Data read from disk, decrypted data will be
                                copied to this page
@param[in]	src_len		source data length
@param[in,out]	dst		Scratch area to use for decryption
@param[in]	dst_len		Size of the scratch area in bytes
@return DB_SUCCESS or error code */
dberr_t Encryption::decrypt_log(const IORequest &type, byte *src, ulint src_len,
                                byte *dst, ulint dst_len) {
  Block *block;
  byte *ptr = src;
  dberr_t ret;

  /* Do nothing if it's not a log request. */
  ut_ad(type.is_log());

  /* The caller doesn't know what to expect */
  if (dst == NULL) {
    block = os_alloc_block();
    dst = block->m_ptr;
  } else {
    block = NULL;
  }

  /* Encrypt the log blocks one by one. */
  while (ptr != src + src_len) {
#ifdef UNIV_ENCRYPT_DEBUG
    {
      std::ostringstream msg;

      ut_print_buf_hex(msg, ptr, OS_FILE_LOG_BLOCK_SIZE);

      ib::info(ER_IB_MSG_847)
          << "Decrypting block: " << log_block_get_hdr_no(ptr) << std::endl
          << "data={" << std::endl
          << msg.str << std::endl
          << "}";
    }
#endif /* UNIV_ENCRYPT_DEBUG */

    /* If it's not an encrypted block, skip it. */
    if (!is_encrypted_log(ptr)) {
      ptr += OS_FILE_LOG_BLOCK_SIZE;
      continue;
    }

    /* Decrypt block */
    ret = decrypt_log_block(type, ptr, dst);
    if (ret != DB_SUCCESS) {
      if (block != NULL) {
        os_free_block(block);
      }

      return (ret);
    }

    ptr += OS_FILE_LOG_BLOCK_SIZE;
  }

  if (block != NULL) {
    os_free_block(block);
  }

  return (DB_SUCCESS);
}

/** Decrypt the page data contents. Page type must be FIL_PAGE_ENCRYPTED,
if not then the source contents are left unchanged and DB_SUCCESS is returned.
@param[in]	type		IORequest
@param[in,out]	src		Data read from disk, decrypted data will be
                                copied to this page
@param[in]	src_len		source data length
@param[in,out]	dst		Scratch area to use for decryption
@param[in]	dst_len		Size of the scratch area in bytes
@return DB_SUCCESS or error code */
dberr_t Encryption::decrypt(const IORequest &type, byte *src, ulint src_len,
                            byte *dst, ulint dst_len) {
  ulint data_len;
  ulint main_len;
  ulint remain_len;
  ulint original_type;
  ulint page_type;
  byte remain_buf[MY_AES_BLOCK_SIZE * 2];
  Block *block;

  if (!is_encrypted_page(src) || m_type == Encryption::NONE) {
    /* There is nothing we can do. */
    return (DB_SUCCESS);
  }

  /* For compressed page, we need to get the compressed size
  for decryption */
  page_type = mach_read_from_2(src + FIL_PAGE_TYPE);
  if (page_type == FIL_PAGE_COMPRESSED_AND_ENCRYPTED) {
    src_len = static_cast<uint16_t>(
                  mach_read_from_2(src + FIL_PAGE_COMPRESS_SIZE_V1)) +
              FIL_PAGE_DATA;
    src_len = ut_calc_align(src_len, type.block_size());
  }

#ifdef UNIV_ENCRYPT_DEBUG
  {
    auto space_id = mach_read_from_4(src + FIL_PAGE_ARCH_LOG_NO_OR_SPACE_ID);

    auto page_no = mach_read_from_4(src + FIL_PAGE_OFFSET);

    std::ostringstream msg;

    msg << "key={";
    ut_print_buf(msg, m_key, 32);
    msg << "}" << std::endl << "iv= {";
    ut_print_buf(msg, m_iv, 32);
    msg << "}";

    ib::info(ER_IB_MSG_848) << "Decrypting page: " << space_id << "." << page_no
                            << " len: " << src_len << "\n"
                            << msg.str();
  }
#endif /* UNIV_ENCRYPT_DEBUG */

  original_type =
      static_cast<uint16_t>(mach_read_from_2(src + FIL_PAGE_ORIGINAL_TYPE_V1));

  byte *ptr = src + FIL_PAGE_DATA;

  /* The caller doesn't know what to expect */
  if (dst == NULL) {
    block = os_alloc_block();
    dst = block->m_ptr;
  } else {
    block = NULL;
  }

  data_len = src_len - FIL_PAGE_DATA;
  main_len = (data_len / MY_AES_BLOCK_SIZE) * MY_AES_BLOCK_SIZE;
  remain_len = data_len - main_len;

  switch (m_type) {
    case Encryption::AES: {
      lint elen;

      /* First decrypt the last 2 blocks data of data, since
      data is no block aligned. */
      if (remain_len != 0) {
        ut_ad(m_klen == ENCRYPTION_KEY_LEN);

        remain_len = MY_AES_BLOCK_SIZE * 2;

        /* Copy the last 2 blocks. */
        memcpy(remain_buf, ptr + data_len - remain_len, remain_len);

        elen = my_aes_decrypt(remain_buf, static_cast<uint32>(remain_len),
                              dst + data_len - remain_len,
                              reinterpret_cast<unsigned char *>(m_key),
                              static_cast<uint32>(m_klen), my_aes_256_cbc,
                              reinterpret_cast<unsigned char *>(m_iv), false);
        if (elen == MY_AES_BAD_DATA) {
          if (block != NULL) {
            os_free_block(block);
          }

          return (DB_IO_DECRYPT_FAIL);
        }

        /* Copy the other data bytes to temp area. */
        memcpy(dst, ptr, data_len - remain_len);
      } else {
        ut_ad(data_len == main_len);

        /* Copy the data bytes to temp area. */
        memcpy(dst, ptr, data_len);
      }

      /* Then decrypt the main data */
      elen = my_aes_decrypt(dst, static_cast<uint32>(main_len), ptr,
                            reinterpret_cast<unsigned char *>(m_key),
                            static_cast<uint32>(m_klen), my_aes_256_cbc,
                            reinterpret_cast<unsigned char *>(m_iv), false);
      if (elen == MY_AES_BAD_DATA) {
        if (block != NULL) {
          os_free_block(block);
        }

        return (DB_IO_DECRYPT_FAIL);
      }

      ut_ad(static_cast<ulint>(elen) == main_len);

      /* Copy the remain bytes. */
      memcpy(ptr + main_len, dst + main_len, data_len - main_len);

      break;
    }

    default:
      if (!type.is_dblwr_recover()) {
        ib::error(ER_IB_MSG_849) << "Encryption algorithm support missing: "
                                 << Encryption::to_string(m_type);
      }

      if (block != NULL) {
        os_free_block(block);
      }

      return (DB_UNSUPPORTED);
  }

  /* Restore the original page type. If it's a compressed and
  encrypted page, just reset it as compressed page type, since
  we will do uncompress later. */

  if (page_type == FIL_PAGE_ENCRYPTED) {
    mach_write_to_2(src + FIL_PAGE_TYPE, original_type);
    mach_write_to_2(src + FIL_PAGE_ORIGINAL_TYPE_V1, 0);
  } else if (page_type == FIL_PAGE_ENCRYPTED_RTREE) {
    mach_write_to_2(src + FIL_PAGE_TYPE, FIL_PAGE_RTREE);
  } else {
    ut_ad(page_type == FIL_PAGE_COMPRESSED_AND_ENCRYPTED);
    mach_write_to_2(src + FIL_PAGE_TYPE, FIL_PAGE_COMPRESSED);
  }

  if (block != NULL) {
    os_free_block(block);
  }

#ifdef UNIV_ENCRYPT_DEBUG
  ib::info(ER_IB_MSG_850) << "Decrypted page: " << space_id << "." << page_no;
#endif /* UNIV_ENCRYPT_DEBUG */

  DBUG_EXECUTE_IF("ib_crash_during_decrypt_page", DBUG_SUICIDE(););

  return (DB_SUCCESS);
}

#ifndef UNIV_HOTBACKUP
/** Check if keyring plugin loaded. */
bool Encryption::check_keyring() {
  size_t key_len;
  bool ret = false;
  char *key_type = nullptr;
  char *master_key = nullptr;
  char key_name[ENCRYPTION_MASTER_KEY_NAME_MAX_LEN];

  key_name[sizeof(ENCRYPTION_DEFAULT_MASTER_KEY)] = 0;

  strncpy(key_name, ENCRYPTION_DEFAULT_MASTER_KEY, sizeof(key_name));

  /* We call key ring API to generate master key here. */
  int my_ret = my_key_generate(key_name, "AES", NULL, ENCRYPTION_KEY_LEN);

  /* We call key ring API to get master key here. */
  my_ret = my_key_fetch(key_name, &key_type, nullptr,
                        reinterpret_cast<void **>(&master_key), &key_len);

  if (my_ret != 0) {
    ib::error(ER_IB_MSG_851) << "Check keyring plugin fail, please check the"
                             << " keyring plugin is loaded.";
  } else {
    my_key_remove(key_name, nullptr);
    ret = true;
  }

  if (key_type != nullptr) {
    my_free(key_type);
  }

  if (master_key != nullptr) {
    my_free(master_key);
  }

  return (ret);
}
#endif /* !UNIV_HOTBACKUP */



