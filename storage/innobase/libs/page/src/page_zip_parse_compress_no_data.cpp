#include <innodb/page/page_zip_parse_compress_no_data.h>

#include <innodb/machine/data.h>
#include <innodb/error/ut_error.h>


ibool page_zip_compress(page_zip_des_t *page_zip, /*!< in: size; out: data,
                                                  n_blobs, m_start, m_end,
                                                  m_nonempty */
                        const page_t *page,       /*!< in: uncompressed page */
                        dict_index_t *index,      /*!< in: index tree */
                        ulint level,              /*!< in: commpression level */
                        mtr_t *mtr);


/** Parses a log record of compressing an index page without the data.
 @return end of log record or NULL */
byte *page_zip_parse_compress_no_data(
    byte *ptr,                /*!< in: buffer */
    byte *end_ptr,            /*!< in: buffer end */
    page_t *page,             /*!< in: uncompressed page */
    page_zip_des_t *page_zip, /*!< out: compressed page */
    dict_index_t *index)      /*!< in: index */
{
  ulint level;
  if (end_ptr == ptr) {
    return (NULL);
  }

  level = mach_read_from_1(ptr);

  /* If page compression fails then there must be something wrong
  because a compress log record is logged only if the compression
  was successful. Crash in this case. */

  if (page && !page_zip_compress(page_zip, page, index, level, NULL)) {
    ut_error;
  }

  return (ptr + 1);
}
