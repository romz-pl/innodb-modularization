#pragma once

#include <innodb/univ/univ.h>


struct Srv_threads {
  /** true if monitor thread is created */
  bool m_monitor_thread_active;

  /** true if error monitor thread is created */
  bool m_error_monitor_thread_active;

  /** true if buffer pool dump/load thread is created */
  bool m_buf_dump_thread_active;

  /** true if buffer pool resize thread is created */
  bool m_buf_resize_thread_active;

  /** true if stats thread is created */
  bool m_dict_stats_thread_active;

  /** true if timeout thread is created */
  bool m_timeout_thread_active;

  /** true if master thread is created */
  bool m_master_thread_active;

  /** true if tablespace alter encrypt thread is created */
  bool m_ts_alter_encrypt_thread_active;
};
