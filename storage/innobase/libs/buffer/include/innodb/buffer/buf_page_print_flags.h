#pragma once

#include <innodb/univ/univ.h>

enum buf_page_print_flags {
  /** Do not crash at the end of buf_page_print(). */
  BUF_PAGE_PRINT_NO_CRASH = 1,
  /** Do not print the full page dump. */
  BUF_PAGE_PRINT_NO_FULL = 2
};
