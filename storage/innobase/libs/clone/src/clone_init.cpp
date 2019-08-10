#include <innodb/clone/clone_init.h>

#include <innodb/clone/Clone_Sys.h>
#include <innodb/io/os_file_status.h>
#include <innodb/clone/clone_sys.h>

dberr_t clone_init() {
  /* Check if in complete cloned data directory */
  os_file_type_t type;
  bool exists = false;
  auto status = os_file_status(CLONE_IN_PROGRESS_FILE, &exists, &type);

  if (status && exists) {
    return (DB_ABORT_INCOMPLETE_CLONE);
  }

  if (clone_sys == nullptr) {
    ut_ad(Clone_Sys::s_clone_sys_state == CLONE_SYS_INACTIVE);
    clone_sys = UT_NEW(Clone_Sys(), mem_key_clone);
  }
  Clone_Sys::s_clone_sys_state = CLONE_SYS_ACTIVE;
  return (DB_SUCCESS);
}
