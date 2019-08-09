#include <innodb/log_write/log_write_encryption.h>

#include <innodb/align/ut_align.h>
#include <innodb/allocator/ut_free.h>
#include <innodb/allocator/ut_malloc_nokey.h>
#include <innodb/disk/univ_page_size.h>
#include <innodb/io/IORequestLogWrite.h>
#include <innodb/log_types/flags.h>
#include <innodb/log_write/log_file_header_fill_encryption.h>
#include <innodb/page/page_id_t.h>
#include <innodb/tablespace/consts.h>
#include <innodb/tablespace/fil_redo_io.h>
#include <innodb/tablespace/fil_space_get.h>
#include <innodb/tablespace/fil_space_t.h>


bool log_write_encryption(byte *key, byte *iv, bool is_boot) {
  const page_id_t page_id{dict_sys_t_s_log_space_first_id, 0};
  byte *log_block_buf_ptr;
  byte *log_block_buf;

  log_block_buf_ptr =
      static_cast<byte *>(ut_malloc_nokey(2 * OS_FILE_LOG_BLOCK_SIZE));
  memset(log_block_buf_ptr, 0, 2 * OS_FILE_LOG_BLOCK_SIZE);
  log_block_buf =
      static_cast<byte *>(ut_align(log_block_buf_ptr, OS_FILE_LOG_BLOCK_SIZE));

  if (key == NULL && iv == NULL) {
    fil_space_t *space = fil_space_get(dict_sys_t_s_log_space_first_id);

    key = space->encryption_key;
    iv = space->encryption_iv;
  }

  if (!log_file_header_fill_encryption(log_block_buf, key, iv, is_boot)) {
    ut_free(log_block_buf_ptr);
    return (false);
  }

  auto err = fil_redo_io(IORequestLogWrite, page_id, univ_page_size,
                         LOG_CHECKPOINT_1 + OS_FILE_LOG_BLOCK_SIZE,
                         OS_FILE_LOG_BLOCK_SIZE, log_block_buf);

  ut_a(err == DB_SUCCESS);

  ut_free(log_block_buf_ptr);
  return (true);
}
