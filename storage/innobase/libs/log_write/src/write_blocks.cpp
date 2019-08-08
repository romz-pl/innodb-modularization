#include <innodb/log_write/write_blocks.h>

#include <innodb/disk/univ_page_size.h>
#include <innodb/io/IORequestLogWrite.h>
#include <innodb/log_redo/srv_log_write_ahead_size.h>
#include <innodb/log_types/log_t.h>
#include <innodb/page/page_id_t.h>
#include <innodb/tablespace/fil_redo_io.h>

namespace Log_files_write_impl {

void write_blocks(log_t &log, byte *write_buf, size_t write_size,
                                uint64_t real_offset) {
  ut_a(write_size >= OS_FILE_LOG_BLOCK_SIZE);
  ut_a(write_size % OS_FILE_LOG_BLOCK_SIZE == 0);
  ut_a(real_offset / UNIV_PAGE_SIZE <= PAGE_NO_MAX);

  page_no_t page_no;

  page_no = static_cast<page_no_t>(real_offset / univ_page_size.physical());

  ut_a(log.write_ahead_end_offset % srv_log_write_ahead_size == 0);

  ut_a(real_offset + write_size <= log.write_ahead_end_offset ||
       (real_offset + write_size) % srv_log_write_ahead_size == 0);

  auto err = fil_redo_io(
      IORequestLogWrite, page_id_t{log.files_space_id, page_no}, univ_page_size,
      static_cast<ulint>(real_offset % UNIV_PAGE_SIZE), write_size, write_buf);

  ut_a(err == DB_SUCCESS);
}

}
