#include <innodb/lock_priv/lock_get_size.h>

#include <innodb/lock_priv/lock_t.h>

/** Gets the size of a lock struct.
 @return size in bytes */
ulint lock_get_size(void)
{
    return ((ulint)sizeof(lock_t));
}
