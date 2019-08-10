#pragma once

#include <innodb/univ/univ.h>

#include <innodb/clone/Clone_Page.h>

/** Comparator for storing sorted page ID. */
struct Less_Clone_Page {
  /** Less than operator for page ID.
  @param[in]	page1	first page
  @param[in]	page2	second page
  @return true, if page1 is less than page2 */
  inline bool operator()(const Clone_Page &page1,
                         const Clone_Page &page2) const {
    if (page1.m_space_id < page2.m_space_id) {
      return (true);
    }

    if (page1.m_space_id == page2.m_space_id &&
        page1.m_page_no < page2.m_page_no) {
      return (true);
    }
    return (false);
  }
};
