#include <innodb/lock_priv/lock_rec_expl_exist_on_page.h>

#include <innodb/lock_sys/lock_mutex_enter.h>
#include <innodb/lock_sys/lock_mutex_exit.h>
#include <innodb/lock_sys/lock_sys.h>
#include <innodb/lock_priv/lock_rec_get_first_on_page_addr.h>

/** Determines if there are explicit record locks on a page.
 @return an explicit record lock on the page, or NULL if there are none */
lock_t *lock_rec_expl_exist_on_page(space_id_t space,  /*!< in: space id */
                                    page_no_t page_no) /*!< in: page number */
{
  lock_t *lock;

  lock_mutex_enter();
  /* Only used in ibuf pages, so rec_hash is good enough */
  lock = lock_rec_get_first_on_page_addr(lock_sys->rec_hash, space, page_no);
  lock_mutex_exit();

  return (lock);
}
