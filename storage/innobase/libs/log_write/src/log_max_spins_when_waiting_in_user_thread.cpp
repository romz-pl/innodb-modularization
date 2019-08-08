#include <innodb/log_write/log_max_spins_when_waiting_in_user_thread.h>

#include <innodb/log_write/srv_cpu_usage.h>
#include <innodb/log_write/srv_log_spin_cpu_pct_hwm.h>
#include <innodb/log_write/srv_log_spin_cpu_abs_lwm.h>


/** Computes maximum number of spin rounds which should be used when waiting
in user thread (for written or flushed redo) or 0 if busy waiting should not
be used at all.
@param[in]  min_non_zero_value    minimum allowed value (unless 0 is returned)
@return maximum number of spin rounds or 0 */
uint64_t log_max_spins_when_waiting_in_user_thread(
    uint64_t min_non_zero_value) {
  uint64_t max_spins;

  /* Get current cpu usage. */
  const double cpu = srv_cpu_usage.utime_pct;

  /* Get high-watermark - when cpu usage is higher, don't spin! */
  const uint32_t hwm = srv_log_spin_cpu_pct_hwm;

  if (srv_cpu_usage.utime_abs < srv_log_spin_cpu_abs_lwm || cpu >= hwm) {
    /* Don't spin because either cpu usage is too high or it's
    almost idle so no reason to bother. */
    max_spins = 0;

  } else if (cpu >= hwm / 2) {
    /* When cpu usage is more than 50% of the hwm, use the minimum allowed
    number of spin rounds, not to increase cpu usage too much (risky). */
    max_spins = min_non_zero_value;

  } else {
    /* When cpu usage is less than 50% of the hwm, choose maximum spin rounds
    in range [minimum, 10*minimum]. Smaller usage of cpu is, more spin rounds
    might be used. */
    const double r = 1.0 * (hwm / 2 - cpu) / (hwm / 2);

    max_spins =
        static_cast<uint64_t>(min_non_zero_value + r * min_non_zero_value * 9);
  }

  return (max_spins);
}


