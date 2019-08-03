#include <innodb/page/page_rec_is_comp.h>

#include <innodb/page/page_is_comp.h>
#include <innodb/page/page_align.h>


/** TRUE if the record is on a page in compact format.
 @return nonzero if in compact format */
ulint page_rec_is_comp(const rec_t *rec) /*!< in: record */
{
  return (page_is_comp(page_align(rec)));
}
