#include <innodb/lock_priv/lock_rec_hash.h>

#include <innodb/lock_priv/lock_rec_fold.h>
#include <innodb/lock_sys/lock_sys.h>
#include <innodb/hash/hash_calc_hash.h>


/** Calculates the hash value of a page file address: used in inserting or
 searching for a lock in the hash table.
 @return hashed value */
ulint lock_rec_hash(space_id_t space,  /*!< in: space */
                    page_no_t page_no) /*!< in: page number */
{
  return (hash_calc_hash(lock_rec_fold(space, page_no), lock_sys->rec_hash));
}
