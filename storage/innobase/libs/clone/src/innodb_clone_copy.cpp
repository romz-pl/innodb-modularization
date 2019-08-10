#include <innodb/clone/innodb_clone_copy.h>

#include <innodb/clone/clone_sys.h>

int innodb_clone_copy(handlerton *hton, THD *thd, const byte *loc, uint loc_len,
                      uint task_id, Ha_clone_cbk *cbk) {
  cbk->set_hton(hton);

  /* Get clone handle by locator index. */
  auto clone_hdl = clone_sys->get_clone_by_index(loc, loc_len);

  auto err = clone_hdl->check_error(thd);
  if (err != 0) {
    return (err);
  }

  /* Start data copy. */
  err = clone_hdl->copy(thd, task_id, cbk);
  clone_hdl->save_error(err);

  return (err);
}
