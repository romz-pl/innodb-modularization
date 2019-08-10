#pragma once

#include <innodb/univ/univ.h>

/** Task information in clone operation. */
struct Clone_Task_Meta {
  /** Index in task array. */
  uint m_task_index;

  /** Current chunk number reserved by the task. */
  uint m_chunk_num;

  /** Current block number that is already transferred. */
  uint m_block_num;
};
