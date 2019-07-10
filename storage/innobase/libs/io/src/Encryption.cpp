#include <innodb/io/Encryption.h>

#include <innodb/allocator/ut_free.h>
#include <innodb/allocator/ut_zalloc_nokey.h>
#include <innodb/assert/assert.h>
#include <innodb/formatting/formatting.h>
#include <innodb/logger/error.h>
#include <innodb/machine/data.h>
#include <innodb/sync_mutex/ib_mutex_t.h>
#include <innodb/sync_mutex/mutex_enter.h>
#include <innodb/sync_mutex/mutex_exit.h>
#include <innodb/page/page_type_t.h>
#include <innodb/page/type.h>

#include "my_aes.h"
#include "my_rnd.h"
#include "sql/mysqld.h"
#include "mysql/service_mysql_keyring.h"




typedef uint32_t (*ut_crc32_func_t)(const byte *ptr, ulint len);

/** Pointer to CRC32 calculation function. */
extern ut_crc32_func_t ut_crc32;


/** Current master key id */
ulint Encryption::s_master_key_id = 0;

/** Current uuid of server instance */
char Encryption::s_uuid[ENCRYPTION_SERVER_UUID_LEN + 1] = {0};


/**
@param[in]      type            The encryption type
@return the string representation */
const char *Encryption::to_string(Type type) {
  switch (type) {
    case NONE:
      return ("N");
    case AES:
      return ("Y");
  }

  ut_ad(0);

  return ("<UNKNOWN>");
}


/** Generate random encryption value for key and iv.
@param[in,out]	value	Encryption value */
void Encryption::random_value(byte *value) {
  ut_ad(value != NULL);

  my_rand_buffer(value, ENCRYPTION_KEY_LEN);
}

/** Create new master key for key rotation.
@param[in,out]	master_key	master key */
void Encryption::create_master_key(byte **master_key) {
#ifndef UNIV_HOTBACKUP
  size_t key_len;
  char *key_type = NULL;
  char key_name[ENCRYPTION_MASTER_KEY_NAME_MAX_LEN];

  /* If uuid does not match with current server uuid,
  set uuid as current server uuid. */
  if (strcmp(s_uuid, server_uuid) != 0) {
    memcpy(s_uuid, server_uuid, sizeof(s_uuid) - 1);
  }

  /* Generate new master key */
  snprintf(key_name, ENCRYPTION_MASTER_KEY_NAME_MAX_LEN, "%s-%s-" ULINTPF,
           ENCRYPTION_MASTER_KEY_PRIFIX, s_uuid, s_master_key_id + 1);

  /* We call key ring API to generate master key here. */
  int ret = my_key_generate(key_name, "AES", nullptr, ENCRYPTION_KEY_LEN);

  /* We call key ring API to get master key here. */
  ret = my_key_fetch(key_name, &key_type, nullptr,
                     reinterpret_cast<void **>(master_key), &key_len);

  if (ret != 0 || *master_key == nullptr) {
    ib::error(ER_IB_MSG_831) << "Encryption can't find master key,"
                             << " please check the keyring plugin is loaded."
                             << " ret=" << ret;

    *master_key = nullptr;
  } else {
    ++s_master_key_id;
  }

  if (key_type != nullptr) {
    my_free(key_type);
  }
#endif /* !UNIV_HOTBACKUP */
}


/** Get master key by key id.
@param[in]	master_key_id	master key id
@param[in]	srv_uuid	uuid of server instance
@param[in,out]	master_key	master key */
void Encryption::get_master_key(ulint master_key_id, char *srv_uuid,
                                byte **master_key) {
  size_t key_len = 0;
  char *key_type = nullptr;
  char key_name[ENCRYPTION_MASTER_KEY_NAME_MAX_LEN];

  memset(key_name, 0x0, sizeof(key_name));

  if (srv_uuid != nullptr) {
    ut_ad(strlen(srv_uuid) > 0);

    snprintf(key_name, ENCRYPTION_MASTER_KEY_NAME_MAX_LEN, "%s-%s-" ULINTPF,
             ENCRYPTION_MASTER_KEY_PRIFIX, srv_uuid, master_key_id);
  } else {
    /* For compitable with 5.7.11, we need to get master key with
    server id. */

    snprintf(key_name, ENCRYPTION_MASTER_KEY_NAME_MAX_LEN, "%s-%lu-" ULINTPF,
             ENCRYPTION_MASTER_KEY_PRIFIX, server_id, master_key_id);
  }

#ifndef UNIV_HOTBACKUP
  /* We call key ring API to get master key here. */
  int ret = my_key_fetch(key_name, &key_type, nullptr,
                         reinterpret_cast<void **>(master_key), &key_len);
#else  /* !UNIV_HOTBACKUP */
  /* We call MEB to get master key here. */
  int ret = meb_key_fetch(key_name, &key_type, NULL,
                          reinterpret_cast<void **>(master_key), &key_len);
#endif /* !UNIV_HOTBACKUP */

  if (key_type != nullptr) {
    my_free(key_type);
  }

  if (ret != 0) {
    *master_key = nullptr;

    ib::error(ER_IB_MSG_832) << "Encryption can't find master key,"
                             << " please check the keyring plugin is loaded.";
  }

#ifdef UNIV_ENCRYPT_DEBUG
  if (ret == 0 && *master_key != nullptr) {
    std::ostringstream msg;

    ut_print_buf(msg, *master_key, key_len);

    ib::info(ER_IB_MSG_833)
        << "Fetched master key: " << master_key_id << "{" << msg.str() << "}";
  }
#endif /* UNIV_ENCRYPT_DEBUG */
}


/** Get current master key and master key id
@param[in,out]	master_key_id	master key id
@param[in,out]	master_key	master key */
void Encryption::get_master_key(ulint *master_key_id, byte **master_key) {
#ifndef UNIV_HOTBACKUP
  int ret;
  size_t key_len;
  char *key_type = nullptr;
  char key_name[ENCRYPTION_MASTER_KEY_NAME_MAX_LEN];
  extern ib_mutex_t master_key_id_mutex;
  bool key_id_locked = false;

  if (s_master_key_id == 0) {
    mutex_enter(&master_key_id_mutex);
    key_id_locked = true;
  }

  memset(key_name, 0x0, sizeof(key_name));

  if (s_master_key_id == 0) {
    ut_ad(strlen(server_uuid) > 0);
    memset(s_uuid, 0x0, sizeof(s_uuid));

    /* If m_master_key is 0, means there's no encrypted
    tablespace, we need to generate the first master key,
    and store it to key ring. */
    memcpy(s_uuid, server_uuid, sizeof(s_uuid) - 1);

    /* Prepare the server s_uuid. */
    snprintf(key_name, ENCRYPTION_MASTER_KEY_NAME_MAX_LEN, "%s-%s-1",
             ENCRYPTION_MASTER_KEY_PRIFIX, s_uuid);

    /* We call key ring API to generate master key here. */
    ret = my_key_generate(key_name, "AES", nullptr, ENCRYPTION_KEY_LEN);

    /* We call key ring API to get master key here. */
    ret = my_key_fetch(key_name, &key_type, nullptr,
                       reinterpret_cast<void **>(master_key), &key_len);

    if (ret == 0 && *master_key != nullptr) {
      ++s_master_key_id;
      *master_key_id = s_master_key_id;
    }
#ifdef UNIV_ENCRYPT_DEBUG
    if (ret == 0 && *master_key != nullptr) {
      std::ostringstream msg;

      ut_print_buf(msg, *master_key, key_len);

      ib::info(ER_IB_MSG_834)
          << "Generated new master key: {" << msg.str() << "}";
    }
#endif /* UNIV_ENCRYPT_DEBUG */
  } else {
    *master_key_id = s_master_key_id;

    snprintf(key_name, ENCRYPTION_MASTER_KEY_NAME_MAX_LEN, "%s-%s-" ULINTPF,
             ENCRYPTION_MASTER_KEY_PRIFIX, s_uuid, *master_key_id);

    /* We call key ring API to get master key here. */
    ret = my_key_fetch(key_name, &key_type, nullptr,
                       reinterpret_cast<void **>(master_key), &key_len);

    /* For compitability with 5.7.11, we need to try to get master
    key with server id when get master key with server uuid
    failure. */
    if (ret != 0 || *master_key == nullptr) {
      if (key_type != nullptr) {
        my_free(key_type);
      }

      snprintf(key_name, ENCRYPTION_MASTER_KEY_NAME_MAX_LEN, "%s-%lu-" ULINTPF,
               ENCRYPTION_MASTER_KEY_PRIFIX, server_id, *master_key_id);

      ret = my_key_fetch(key_name, &key_type, nullptr,
                         reinterpret_cast<void **>(master_key), &key_len);
    }

#ifdef UNIV_ENCRYPT_DEBUG
    if (ret == 0 && *master_key != nullptr) {
      std::ostringstream msg;

      ut_print_buf(msg, *master_key, key_len);

      ib::info(ER_IB_MSG_835) << "Fetched master key: " << *master_key_id
                              << ": {" << msg.str() << "}";
    }
#endif /* UNIV_ENCRYPT_DEBUG */
  }

  if (ret != 0) {
    *master_key = nullptr;
    ib::error(ER_IB_MSG_836) << "Encryption can't find master key, please check"
                             << " the keyring plugin is loaded.";
  }

  if (key_type != nullptr) {
    my_free(key_type);
  }

  if (key_id_locked) {
    mutex_exit(&master_key_id_mutex);
  }

#endif /* !UNIV_HOTBACKUP */
}

/** Fill the encryption information.
@param[in]	key		encryption key
@param[in]	iv		encryption iv
@param[in,out]	encrypt_info	encryption information
@param[in]	is_boot		if it's for bootstrap
@return true if success */
bool Encryption::fill_encryption_info(byte *key, byte *iv, byte *encrypt_info,
                                      bool is_boot) {
  byte *master_key = nullptr;
  ulint master_key_id;
  bool is_default_master_key = false;

  /* Get master key from key ring. For bootstrap, we use a default
  master key which master_key_id is 0. */
  if (is_boot
#ifndef UNIV_HOTBACKUP
      || (strlen(server_uuid) == 0)
#endif
  ) {
    master_key_id = 0;

    master_key = static_cast<byte *>(ut_zalloc_nokey(ENCRYPTION_KEY_LEN));

    ut_ad(ENCRYPTION_KEY_LEN >= sizeof(ENCRYPTION_DEFAULT_MASTER_KEY));

    strcpy(reinterpret_cast<char *>(master_key), ENCRYPTION_DEFAULT_MASTER_KEY);
    is_default_master_key = true;
  } else {
    get_master_key(&master_key_id, &master_key);

    if (master_key == nullptr) {
      return (false);
    }
  }

  memset(encrypt_info, 0, ENCRYPTION_INFO_SIZE);

  /* Use the new master key to encrypt the key. */
  ut_ad(encrypt_info != nullptr);
  auto ptr = encrypt_info;

  memcpy(ptr, ENCRYPTION_KEY_MAGIC_V3, ENCRYPTION_MAGIC_SIZE);

  ptr += ENCRYPTION_MAGIC_SIZE;

  /* Write master key id. */
  mach_write_to_4(ptr, master_key_id);
  ptr += sizeof(uint32);

  /* Write server uuid. */
  memcpy(reinterpret_cast<char *>(ptr), s_uuid, sizeof(s_uuid));
  ptr += sizeof(s_uuid) - 1;

  byte key_info[ENCRYPTION_KEY_LEN * 2];

  memset(key_info, 0x0, sizeof(key_info));

  memcpy(key_info, key, ENCRYPTION_KEY_LEN);

  memcpy(key_info + ENCRYPTION_KEY_LEN, iv, ENCRYPTION_KEY_LEN);

  /* Encrypt key and iv. */
  auto elen =
      my_aes_encrypt(key_info, sizeof(key_info), ptr, master_key,
                     ENCRYPTION_KEY_LEN, my_aes_256_ecb, nullptr, false);

  if (elen == MY_AES_BAD_DATA) {
    my_free(master_key);
    return (false);
  }

  ptr += sizeof(key_info);

  /* Write checksum bytes. */
  auto crc = ut_crc32(key_info, sizeof(key_info));

  mach_write_to_4(ptr, crc);

  if (is_default_master_key) {
    ut_free(master_key);
  } else {
    my_free(master_key);
  }

  return (true);
}

/** Get master key from encryption information
@param[in]	encrypt_info	encryption information
@param[in]	version		version of encryption information
@param[in,out]	m_key_id	master key id
@param[in,out]	srv_uuid	server uuid
@param[in,out]	master_key	master key
@return position after master key id or uuid, or the old position
if can't get the master key. */
byte *Encryption::get_master_key_from_info(byte *encrypt_info, Version version,
                                           uint32_t *m_key_id, char *srv_uuid,
                                           byte **master_key) {
  byte *ptr;
  uint32 key_id;

  ptr = encrypt_info;
  *m_key_id = 0;

  /* Get master key id. */
  key_id = mach_read_from_4(ptr);
  ptr += sizeof(uint32);

  /* Handle different version encryption information. */
  switch (version) {
    case ENCRYPTION_VERSION_1:
      /* For version 1, it's possible master key id
      occupied 8 bytes. */
      if (mach_read_from_4(ptr) == 0) {
        ptr += sizeof(uint32);
      }

      get_master_key(key_id, nullptr, master_key);
      if (*master_key == nullptr) {
        return (encrypt_info);
      }

      *m_key_id = key_id;
      return (ptr);

    case ENCRYPTION_VERSION_2:
      /* For version 2, it's also possible master key id
      occupied 8 bytes. */
      if (mach_read_from_4(ptr) == 0) {
        ptr += sizeof(uint32);
      }

      /* Get server uuid. */
      memset(srv_uuid, 0, ENCRYPTION_SERVER_UUID_LEN + 1);
      memcpy(srv_uuid, ptr, ENCRYPTION_SERVER_UUID_LEN);

      ut_ad(strlen(srv_uuid) != 0);
      ptr += ENCRYPTION_SERVER_UUID_LEN;

      /* Get master key. */
      get_master_key(key_id, srv_uuid, master_key);
      if (*master_key == nullptr) {
        return (encrypt_info);
      }

      *m_key_id = key_id;
      break;

    case ENCRYPTION_VERSION_3:
      /* Get server uuid. */
      memset(srv_uuid, 0, ENCRYPTION_SERVER_UUID_LEN + 1);
      memcpy(srv_uuid, ptr, ENCRYPTION_SERVER_UUID_LEN);

      ptr += ENCRYPTION_SERVER_UUID_LEN;

      if (key_id == 0) {
        /* When key_id is 0, which means it's the
        default master key for bootstrap. */
        *master_key = static_cast<byte *>(ut_zalloc_nokey(ENCRYPTION_KEY_LEN));
        memcpy(*master_key, ENCRYPTION_DEFAULT_MASTER_KEY,
               strlen(ENCRYPTION_DEFAULT_MASTER_KEY));
        *m_key_id = 0;
      } else {
        ut_ad(strlen(srv_uuid) != 0);

        /* Get master key. */
        get_master_key(key_id, srv_uuid, master_key);
        if (*master_key == nullptr) {
          return (encrypt_info);
        }

        *m_key_id = key_id;
      }
      break;
  }

  ut_ad(*master_key != nullptr);

  return (ptr);
}



/** Check if page is encrypted page or not
@param[in]	page	page which need to check
@return true if it is a encrypted page */
bool Encryption::is_encrypted_page(const byte *page) {
  ulint page_type = mach_read_from_2(page + FIL_PAGE_TYPE);

  return (page_type == FIL_PAGE_ENCRYPTED ||
          page_type == FIL_PAGE_COMPRESSED_AND_ENCRYPTED ||
          page_type == FIL_PAGE_ENCRYPTED_RTREE);
}


