#include <innodb/log_write/log_read_encryption.h>

#include <innodb/allocator/ut_malloc_nokey.h>
#include <innodb/disk/univ_page_size.h>
#include <innodb/io/IORequestLogRead.h>
#include <innodb/log_redo/flags.h>
#include <innodb/log_types/log_t.h>
#include <innodb/logger/info.h>
#include <innodb/page/page_id_t.h>
#include <innodb/tablespace/consts.h>
#include <innodb/tablespace/fil_redo_io.h>
#include <innodb/tablespace/fil_set_encryption.h>
#include <innodb/tablespace/fil_space_get.h>
#include <innodb/tablespace/fil_space_t.h>

bool log_read_encryption() {
  space_id_t log_space_id = dict_sys_t_s_log_space_first_id;
  const page_id_t page_id(log_space_id, 0);
  byte *log_block_buf_ptr;
  byte *log_block_buf;
  byte key[ENCRYPTION_KEY_LEN];
  byte iv[ENCRYPTION_KEY_LEN];
  fil_space_t *space = fil_space_get(log_space_id);
  dberr_t err;

  log_block_buf_ptr =
      static_cast<byte *>(ut_malloc_nokey(2 * OS_FILE_LOG_BLOCK_SIZE));
  memset(log_block_buf_ptr, 0, 2 * OS_FILE_LOG_BLOCK_SIZE);
  log_block_buf =
      static_cast<byte *>(ut_align(log_block_buf_ptr, OS_FILE_LOG_BLOCK_SIZE));

  err = fil_redo_io(IORequestLogRead, page_id, univ_page_size,
                    LOG_CHECKPOINT_1 + OS_FILE_LOG_BLOCK_SIZE,
                    OS_FILE_LOG_BLOCK_SIZE, log_block_buf);

  ut_a(err == DB_SUCCESS);

  if (memcmp(log_block_buf + LOG_HEADER_CREATOR_END, ENCRYPTION_KEY_MAGIC_V3,
             ENCRYPTION_MAGIC_SIZE) == 0) {
    /* Make sure the keyring is loaded. */
    if (!Encryption::check_keyring()) {
      ut_free(log_block_buf_ptr);
      ib::error(ER_IB_MSG_1238) << "Redo log was encrypted,"
                                << " but keyring plugin is not loaded.";
      return (false);
    }

    if (Encryption::decode_encryption_info(
            key, iv, log_block_buf + LOG_HEADER_CREATOR_END)) {
      /* If redo log encryption is enabled, set the
      space flag. Otherwise, we just fill the encryption
      information to space object for decrypting old
      redo log blocks. */
      fsp_flags_set_encryption(space->flags);
      err = fil_set_encryption(space->id, Encryption::AES, key, iv);

      if (err == DB_SUCCESS) {
        ut_free(log_block_buf_ptr);
        ib::info(ER_IB_MSG_1239) << "Read redo log encryption"
                                 << " metadata successful.";
        return (true);
      } else {
        ut_free(log_block_buf_ptr);
        ib::error(ER_IB_MSG_1240) << "Can't set redo log tablespace"
                                  << " encryption metadata.";
        return (false);
      }
    } else {
      ut_free(log_block_buf_ptr);
      ib::error(ER_IB_MSG_1241) << "Cannot read the encryption"
                                   " information in log file header, please"
                                   " check if keyring plugin loaded and"
                                   " the key file exists.";
      return (false);
    }
  }

  ut_free(log_block_buf_ptr);
  return (true);
}
