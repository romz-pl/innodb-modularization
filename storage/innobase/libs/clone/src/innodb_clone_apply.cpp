#include <innodb/clone/innodb_clone_apply.h>

#include <innodb/clone/clone_sys.h>

int innodb_clone_apply(handlerton *hton, THD *thd, const byte *loc,
                       uint loc_len, uint task_id, int in_err,
                       Ha_clone_cbk *cbk) {
  /* Get clone handle by locator index. */
  auto clone_hdl = clone_sys->get_clone_by_index(loc, loc_len);
  ut_ad(in_err != 0 || cbk != nullptr);

  /* For error input, return after saving it */
  if (in_err != 0 || cbk == nullptr) {
    clone_hdl->save_error(in_err);
    ib::info(ER_IB_MSG_151) << "Clone Apply set error code: " << in_err;
    return (0);
  }

  cbk->set_hton(hton);
  auto err = clone_hdl->check_error(thd);
  if (err != 0) {
    return (err);
  }

  /* Apply data received from callback. */
  err = clone_hdl->apply(thd, task_id, cbk);
  clone_hdl->save_error(err);

  return (err);
}
