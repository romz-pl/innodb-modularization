#include <innodb/clone/clone_mark_active.h>

#include <innodb/clone/clone_sys.h>

void clone_mark_active() {
  mutex_enter(clone_sys->get_mutex());

  clone_sys->mark_active();

  mutex_exit(clone_sys->get_mutex());
}
