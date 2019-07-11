#include <innodb/tablespace/page_id_t.h>

#include <ostream>

/** Print the given page_id_t object.
@param[in,out]	out	the output stream
@param[in]	page_id	the page_id_t object to be printed
@return the output stream */
std::ostream &operator<<(std::ostream &out, const page_id_t &page_id) {
  out << "[page id: space=" << page_id.m_space
      << ", page number=" << page_id.m_page_no << "]";
  return (out);
}
