#include <innodb/clone/innodb_clone_apply_end.h>

#include <innodb/clone/innodb_clone_end.h>

int innodb_clone_apply_end(handlerton *hton, THD *thd, const byte *loc,
                           uint loc_len, uint task_id, int in_err) {
  auto err = innodb_clone_end(hton, thd, loc, loc_len, task_id, in_err);
  return (err);
}
