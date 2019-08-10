#pragma once

#include <innodb/univ/univ.h>

/** Page identified by space and page number */
struct Clone_Page {
  /** Tablespace ID */
  ib_uint32_t m_space_id;

  /** Page number within tablespace */
  ib_uint32_t m_page_no;
};
