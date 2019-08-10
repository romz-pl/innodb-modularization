#include <innodb/clone/clone_mark_abort.h>

#include <innodb/clone/clone_sys.h>

bool clone_mark_abort(bool force) {
  bool aborted;

  mutex_enter(clone_sys->get_mutex());

  aborted = clone_sys->mark_abort(force);

  mutex_exit(clone_sys->get_mutex());

  DEBUG_SYNC_C("clone_marked_abort2");

  return (aborted);
}
