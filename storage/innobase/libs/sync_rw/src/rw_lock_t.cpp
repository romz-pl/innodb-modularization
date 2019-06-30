#include <innodb/sync_rw/rw_lock_t.h>

#ifdef UNIV_DEBUG

/** Print where it was locked from
@return the string representation */
std::string rw_lock_t::locked_from() const {
  /* Note: For X locks it can be locked form multiple places because
  the same thread can call X lock recursively. */

  std::ostringstream msg;
  Infos infos;

  rw_lock_get_debug_info(this, &infos);

  ulint i = 0;
  Infos::const_iterator end = infos.end();

  for (Infos::const_iterator it = infos.begin(); it != end; ++it, ++i) {
    const rw_lock_debug_t *info = *it;

    ut_ad(os_thread_eq(info->thread_id, os_thread_get_curr_id()));

    if (i > 0) {
      msg << ", ";
    }

    msg << info->file_name << ":" << info->line;
  }

  return (msg.str());
}

/** Print the rw-lock information.
@return the string representation */
std::string rw_lock_t::to_string() const {
  std::ostringstream msg;

  msg << "RW-LATCH: "
      << "thread id " << os_thread_get_curr_id() << " addr: " << this
      << " Locked from: " << locked_from().c_str();

  return (msg.str());
}
#endif /* UNIV_DEBUG */
