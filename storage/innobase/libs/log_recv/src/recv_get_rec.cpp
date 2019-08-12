#include <innodb/log_recv/recv_get_rec.h>

#include <innodb/log_recv/recv_sys_t.h>
#include <innodb/log_recv/recv_get_page_map.h>

/** Gets the list of log records for a <space, page>.
@param[in]	space_id	Tablespace ID
@param[in]	page_no		Page number
@return the redo log entries or nullptr if not found */
recv_addr_t *recv_get_rec(space_id_t space_id, page_no_t page_no) {
  recv_sys_t::Space *space;

  space = recv_get_page_map(space_id, false);

  if (space != nullptr) {
    auto it = space->m_pages.find(page_no);

    if (it != space->m_pages.end()) {
      return (it->second);
    }
  }

  return (nullptr);
}
