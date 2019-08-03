#include <innodb/record/rec_get_node_ptr_flag.h>

#include <innodb/record/rec_get_status.h>
#include <innodb/record/flag.h>

/** The following function tells if a new-style record is a node pointer.
 @return true if node pointer */
bool rec_get_node_ptr_flag(const rec_t *rec) /*!< in: physical record */
{
  return (REC_STATUS_NODE_PTR == rec_get_status(rec));
}
