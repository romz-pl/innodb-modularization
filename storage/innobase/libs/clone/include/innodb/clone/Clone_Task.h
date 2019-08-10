#pragma once

#include <innodb/univ/univ.h>

#include <innodb/clone/Clone_Task_Meta.h>
#include <innodb/clone/Clone_Task_State.h>
#include <innodb/io/pfs_os_file_t.h>

/** Task for clone operation. Multiple task can concurrently work
on a clone operation. */
struct Clone_Task {
  /** Task Meta data */
  Clone_Task_Meta m_task_meta;

  /** Task state */
  Clone_Task_State m_task_state;

  /** Serial descriptor byte string */
  byte *m_serial_desc;

  /** Serial descriptor allocated length */
  uint m_alloc_len;

  /** Current file descriptor */
  pfs_os_file_t m_current_file_des;

  /** Current file index */
  uint m_current_file_index;

  /** Data files are read using OS buffer cache */
  bool m_file_cache;

  /** If master task */
  bool m_is_master;

  /** If task has associated session */
  bool m_has_thd;

#ifdef UNIV_DEBUG
  /** Ignore debug sync point */
  bool m_ignore_sync;

  /** Counter to restart in different state */
  int m_debug_counter;
#endif /* UNIV_DEBUG */

  /** Allocated buffer */
  byte *m_current_buffer;

  /** Allocated buffer length */
  uint m_buffer_alloc_len;

  /** Data transferred for current chunk in bytes */
  uint32_t m_data_size;
};
