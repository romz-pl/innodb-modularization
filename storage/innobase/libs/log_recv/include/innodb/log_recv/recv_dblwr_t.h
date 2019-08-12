#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_no_t.h>
#include <innodb/allocator/ut_free.h>

#include <list>

struct recv_dblwr_t {
  // Default constructor
  recv_dblwr_t() : deferred(), pages() {}

  /** Add a page frame to the doublewrite recovery buffer. */
  void add(const byte *page) { pages.push_back(page); }

  /** Find a doublewrite copy of a page.
  @param[in]	space_id	tablespace identifier
  @param[in]	page_no		page number
  @return	page frame
  @retval NULL if no page was found */
  const byte *find_page(space_id_t space_id, page_no_t page_no);

  using List = std::list<const byte *>;

  struct Page {
    /** Default constructor */
    Page() : m_no(), m_ptr(), m_page() {}

    /** Constructor
    @param[in]	no	Doublewrite page number
    @param[in]	page	Page read from no */
    Page(page_no_t no, const byte *page);

    /** Free the memory */
    void close() {
      ut_free(m_ptr);
      m_ptr = nullptr;
      m_page = nullptr;
    }

    /** Page number if the doublewrite buffer */
    page_no_t m_no;

    /** Unaligned pointer */
    byte *m_ptr;

    /** Aligned pointer derived from ptr */
    byte *m_page;
  };

  using Deferred = std::list<Page>;

  /** Pages that could not be recovered from the doublewrite
  buffer at the start and need to be recovered once we process an
  MLOG_FILE_OPEN redo log record */
  Deferred deferred;

  /** Recovered doublewrite buffer page frames */
  List pages;

  // Disable copying
  recv_dblwr_t(const recv_dblwr_t &) = delete;
  recv_dblwr_t &operator=(const recv_dblwr_t &) = delete;
};
