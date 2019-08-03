#include <innodb/page/page_zip_empty_size.h>

#include <zlib.h>

#include <innodb/page/flag.h>
#include <innodb/page/header.h>
#include <innodb/record/flag.h>

/** Determine the guaranteed free space on an empty page.
 @return minimum payload size on the page */
ulint page_zip_empty_size(
    ulint n_fields, /*!< in: number of columns in the index */
    ulint zip_size) /*!< in: compressed page size in bytes */
{
  lint size = zip_size
              /* subtract the page header and the longest
              uncompressed data needed for one record */
              - (PAGE_DATA + PAGE_ZIP_CLUST_LEAF_SLOT_SIZE +
                 1   /* encoded heap_no==2 in page_zip_write_rec() */
                 + 1 /* end of modification log */
                 - REC_N_NEW_EXTRA_BYTES /* omitted bytes */)
              /* subtract the space for page_zip_fields_encode() */
              - compressBound(static_cast<uLong>(2 * (n_fields + 1)));
  return (size > 0 ? (ulint)size : 0);
}
