#include <innodb/srv_thread/srv_thread_type_validate.h>

#ifdef UNIV_DEBUG
/** Validates the type of a thread table slot.
 @return true if ok */
static ibool srv_thread_type_validate(
    srv_thread_type type) /*!< in: thread type */
{
  switch (type) {
    case SRV_NONE:
      break;
    case SRV_WORKER:
    case SRV_PURGE:
    case SRV_MASTER:
      return (TRUE);
  }
  ut_error;
}
#endif /* UNIV_DEBUG */
