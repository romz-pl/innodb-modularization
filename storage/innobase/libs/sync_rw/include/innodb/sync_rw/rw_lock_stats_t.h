#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/counter/counter.h>

/** Counters for RW locks. */
struct rw_lock_stats_t {
  typedef ib_counter_t<uint64_t, IB_N_SLOTS> uint64_counter_t;

  /** number of spin waits on rw-latches,
  resulted during shared (read) locks */
  uint64_counter_t rw_s_spin_wait_count;

  /** number of spin loop rounds on rw-latches,
  resulted during shared (read) locks */
  uint64_counter_t rw_s_spin_round_count;

  /** number of OS waits on rw-latches,
  resulted during shared (read) locks */
  uint64_counter_t rw_s_os_wait_count;

  /** number of spin waits on rw-latches,
  resulted during exclusive (write) locks */
  uint64_counter_t rw_x_spin_wait_count;

  /** number of spin loop rounds on rw-latches,
  resulted during exclusive (write) locks */
  uint64_counter_t rw_x_spin_round_count;

  /** number of OS waits on rw-latches,
  resulted during exclusive (write) locks */
  uint64_counter_t rw_x_os_wait_count;

  /** number of spin waits on rw-latches,
  resulted during sx locks */
  uint64_counter_t rw_sx_spin_wait_count;

  /** number of spin loop rounds on rw-latches,
  resulted during sx locks */
  uint64_counter_t rw_sx_spin_round_count;

  /** number of OS waits on rw-latches,
  resulted during sx locks */
  uint64_counter_t rw_sx_os_wait_count;
};

#endif
