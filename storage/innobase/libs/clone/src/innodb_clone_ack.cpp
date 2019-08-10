#include <innodb/clone/innodb_clone_ack.h>

#include <innodb/clone/clone_validate_locator.h>
#include <innodb/clone/clone_sys.h>

int innodb_clone_ack(handlerton *hton, THD *thd, const byte *loc, uint loc_len,
                     uint task_id, int in_err, Ha_clone_cbk *cbk) {
  cbk->set_hton(hton);

  /* Check if reference locator is valid */
  if (loc != nullptr && !clone_validate_locator(loc, loc_len)) {
    int err = ER_CLONE_PROTOCOL_ERROR;
    my_error(err, MYF(0), "Wrong Clone RPC: Invalid Locator");
    return (err);
  }
  mutex_enter(clone_sys->get_mutex());

  /* Find attach clone handle using the reference locator. */
  auto clone_hdl = clone_sys->find_clone(loc, loc_len, CLONE_HDL_COPY);

  mutex_exit(clone_sys->get_mutex());

  /* Must find existing clone for the locator */
  if (clone_hdl == nullptr) {
    my_error(ER_INTERNAL_ERROR, MYF(0),

             "Innodb Clone ACK refers non-existing clone");
    return (ER_INTERNAL_ERROR);
  }

  int err = 0;

  if (in_err == 0) {
    /* Apply acknowledged data */
    err = clone_hdl->apply(thd, task_id, cbk);

    clone_hdl->save_error(err);
  } else {
    /* For error input, return after saving it */
    ib::info(ER_IB_MSG_151) << "Clone set error ACK: " << in_err;
    clone_hdl->save_error(in_err);
  }

  mutex_enter(clone_sys->get_mutex());

  /* Detach from clone handle */
  clone_sys->drop_clone(clone_hdl);

  mutex_exit(clone_sys->get_mutex());

  return (err);
}
