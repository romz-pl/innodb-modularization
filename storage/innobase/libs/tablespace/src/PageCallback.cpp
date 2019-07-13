#include <innodb/tablespace/PageCallback.h>

#include <innodb/tablespace/fsp_header_get_page_size.h>


/** Set the tablespace table size.
@param[in]	page	a page belonging to the tablespace */
void PageCallback::set_page_size(const buf_frame_t *page) UNIV_NOTHROW {
  m_page_size.copy_from(fsp_header_get_page_size(page));
}
