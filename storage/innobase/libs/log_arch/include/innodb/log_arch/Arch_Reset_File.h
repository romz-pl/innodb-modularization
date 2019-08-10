#pragma once

#include <innodb/univ/univ.h>

#include <innodb/log_sn/lsn_t.h>
#include <innodb/log_arch/Arch_Point.h>

#include <vector>

/* Structure which represents a file in a group and its reset points. */
struct Arch_Reset_File {
  /* Initialize the structure. */
  void init();

  /* Index of the file in the group */
  uint m_file_index{0};

  /* LSN of the first reset point in the vector of reset points this
  structure maintains. Treated as the file LSN. */
  lsn_t m_lsn{LSN_MAX};

  /* Vector of reset points which belong to this file */
  std::vector<Arch_Point> m_start_point;
};
