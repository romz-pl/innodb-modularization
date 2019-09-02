#include <innodb/srv_thread/srv_slot_get_type.h>

#include <innodb/srv_thread/srv_slot_t.h>
#include <innodb/srv_thread/srv_thread_type_validate.h>
#include <innodb/assert/assert.h>


/** Gets the type of a thread table slot.
 @return thread type */
srv_thread_type srv_slot_get_type(
    const srv_slot_t *slot) /*!< in: thread slot */
{
  srv_thread_type type = slot->type;
  ut_ad(srv_thread_type_validate(type));
  return (type);
}
