#include <innodb/buffer/buf_pool_is_obsolete.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/buffer/buf_pool_withdrawing.h>
#include <innodb/buffer/buf_withdraw_clock.h>

/** Verify the possibility that a stored page is not in buffer pool.
@param[in]	withdraw_clock	withdraw clock when stored the page
@retval true	if the page might be relocated */
bool buf_pool_is_obsolete(ulint withdraw_clock) {
  return (buf_pool_withdrawing || buf_withdraw_clock != withdraw_clock);
}

#endif
