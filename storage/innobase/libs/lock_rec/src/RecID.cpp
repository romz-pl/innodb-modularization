#include <innodb/lock_rec/RecID.h>

#include <innodb/lock_priv/lock_t.h>
#include <innodb/lock_priv/lock_rec_get_nth_bit.h>


/* Check if the rec id matches the lock instance.
@param[i]	lock		Lock to compare with
@return true if <space, page_no, heap_no> matches the lock. */
bool RecID::matches(const lock_t *lock) const {
  return (lock->rec_lock.space == m_space_id &&
          lock->rec_lock.page_no == m_page_no &&
          lock_rec_get_nth_bit(lock, m_heap_no));
}
