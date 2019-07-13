#include <innodb/sync_rw/sync_print_wait_info.h>

#include <innodb/formatting/formatting.h>
#include <innodb/sync_rw/rw_lock_stats.h>

#include <utility>

/**
Prints wait info of the sync system.
@param file - where to print */
void sync_print_wait_info(FILE *file) {
  fprintf(file,
          "RW-shared spins " UINT64PF ", rounds " UINT64PF
          ","
          " OS waits " UINT64PF
          "\n"
          "RW-excl spins " UINT64PF ", rounds " UINT64PF
          ","
          " OS waits " UINT64PF
          "\n"
          "RW-sx spins " UINT64PF ", rounds " UINT64PF
          ","
          " OS waits " UINT64PF "\n",
          (uint64_t)rw_lock_stats.rw_s_spin_wait_count,
          (uint64_t)rw_lock_stats.rw_s_spin_round_count,
          (uint64_t)rw_lock_stats.rw_s_os_wait_count,
          (uint64_t)rw_lock_stats.rw_x_spin_wait_count,
          (uint64_t)rw_lock_stats.rw_x_spin_round_count,
          (uint64_t)rw_lock_stats.rw_x_os_wait_count,
          (uint64_t)rw_lock_stats.rw_sx_spin_wait_count,
          (uint64_t)rw_lock_stats.rw_sx_spin_round_count,
          (uint64_t)rw_lock_stats.rw_sx_os_wait_count);

  fprintf(
      file,
      "Spin rounds per wait: %.2f RW-shared,"
      " %.2f RW-excl, %.2f RW-sx\n",
      (double)rw_lock_stats.rw_s_spin_round_count /
          std::max(uint64_t(1), (uint64_t)rw_lock_stats.rw_s_spin_wait_count),
      (double)rw_lock_stats.rw_x_spin_round_count /
          std::max(uint64_t(1), (uint64_t)rw_lock_stats.rw_x_spin_wait_count),
      (double)rw_lock_stats.rw_sx_spin_round_count /
          std::max(uint64_t(1), (uint64_t)rw_lock_stats.rw_sx_spin_wait_count));
}
