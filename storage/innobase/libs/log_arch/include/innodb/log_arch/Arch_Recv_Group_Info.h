#pragma once

#include <innodb/univ/univ.h>

#include <limits>

/** Information of a group required for parsing information from the archived
file. */
struct Arch_Recv_Group_Info {
  /** Number of archived files belonging to the group. */
  uint m_num_files{0};

  /** Group is active or not. */
  bool m_active{false};

  /** True if group is from durable archiving, false if left over from a crash
   * during clone operation. */
  bool m_durable{false};

  /** The file index which is part of the file name may not necessarily
  be 0 always. It's possible that purge might have purged files in the
  group leading to the file index of the first file in the group being
  greater than 0. So we need this info to know the index of the first
  file in the group. */
  uint m_file_start_index{std::numeric_limits<uint>::max()};
};
