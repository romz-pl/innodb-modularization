#include <innodb/log_files/log_files_header_read.h>

#include <innodb/log_types/log_t.h>
#include <innodb/tablespace/fil_redo_io.h>
#include <innodb/io/IORequestLogRead.h>
#include <innodb/io/srv_is_being_started.h>
#include <innodb/disk/univ_page_size.h>
#include <innodb/page/page_id_t.h>

void log_files_header_read(log_t &log, uint32_t header) {
  ut_a(srv_is_being_started);
  ut_a(!log.checkpointer_thread_alive.load());

  const auto page_no =
      static_cast<page_no_t>(header / univ_page_size.physical());

  auto err = fil_redo_io(IORequestLogRead,
                         page_id_t{log.files_space_id, page_no}, univ_page_size,
                         static_cast<ulint>(header % univ_page_size.physical()),
                         OS_FILE_LOG_BLOCK_SIZE, log.checkpoint_buf);

  ut_a(err == DB_SUCCESS);
}
