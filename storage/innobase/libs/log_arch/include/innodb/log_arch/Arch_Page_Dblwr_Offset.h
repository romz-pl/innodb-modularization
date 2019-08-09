#pragma once

#include <innodb/univ/univ.h>

/** Page Archive doublewrite buffer block offsets */
enum Arch_Page_Dblwr_Offset {
  /** Archive doublewrite buffer page offset for RESET page. */
  ARCH_PAGE_DBLWR_RESET_PAGE = 0,

  /* Archive doublewrite buffer page offset for FULL FLUSH page. */
  ARCH_PAGE_DBLWR_FULL_FLUSH_PAGE,

  /* Archive doublewrite buffer page offset for PARTIAL FLUSH page. */
  ARCH_PAGE_DBLWR_PARTIAL_FLUSH_PAGE
};
