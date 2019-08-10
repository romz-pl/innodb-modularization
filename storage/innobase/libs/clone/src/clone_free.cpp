#include <innodb/clone/clone_free.h>

#include <innodb/clone/Clone_Sys.h>
#include <innodb/clone/clone_sys.h>

void clone_free() {
  if (clone_sys != nullptr) {
    ut_ad(Clone_Sys::s_clone_sys_state == CLONE_SYS_ACTIVE);

    UT_DELETE(clone_sys);
    clone_sys = nullptr;
  }

  Clone_Sys::s_clone_sys_state = CLONE_SYS_INACTIVE;
}
