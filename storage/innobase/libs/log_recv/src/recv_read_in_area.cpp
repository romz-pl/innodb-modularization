#include <innodb/log_recv/recv_read_in_area.h>

#include <innodb/log_recv/recv_get_rec.h>
#include <innodb/sync_mutex/mutex_enter.h>
#include <innodb/sync_mutex/mutex_exit.h>
#include <innodb/log_recv/flags.h>
#include <innodb/log_recv/recv_sys.h>

#include <array>

void buf_read_recv_pages(bool sync, space_id_t space_id,
                         const page_no_t *page_nos, ulint n_stored);

ibool buf_page_peek(const page_id_t &page_id);

/** Reads in pages which have hashed log records, from an area around a given
page number.
@param[in]	page_id		Read the pages around this page number
@return number of pages found */
ulint recv_read_in_area(const page_id_t &page_id) {
  page_no_t low_limit;

  low_limit = page_id.page_no() - (page_id.page_no() % RECV_READ_AHEAD_AREA);

  ulint n = 0;

  std::array<page_no_t, RECV_READ_AHEAD_AREA> page_nos;

  for (page_no_t page_no = low_limit;
       page_no < low_limit + RECV_READ_AHEAD_AREA; ++page_no) {
    recv_addr_t *recv_addr;

    recv_addr = recv_get_rec(page_id.space(), page_no);

    const page_id_t cur_page_id(page_id.space(), page_no);

    if (recv_addr != nullptr && !buf_page_peek(cur_page_id)) {
      mutex_enter(&recv_sys->mutex);

      if (recv_addr->state == RECV_NOT_PROCESSED) {
        recv_addr->state = RECV_BEING_READ;

        page_nos[n] = page_no;

        ++n;
      }

      mutex_exit(&recv_sys->mutex);
    }
  }

  buf_read_recv_pages(false, page_id.space(), &page_nos[0], n);

  return (n);
}
