#pragma once

#include <innodb/univ/univ.h>

struct dict_index_t;
struct buf_block_t;
struct mtr_t;

/** Reorganize and compress a page.  This is a low-level operation for
 compressed pages, to be used when page_zip_compress() fails.
 On success, a redo log entry MLOG_ZIP_PAGE_COMPRESS will be written.
 The function btr_page_reorganize() should be preferred whenever possible.
 IMPORTANT: if page_zip_reorganize() is invoked on a leaf page of a
 non-clustered index, the caller must update the insert buffer free
 bits in the same mini-transaction in such a way that the modification
 will be redo-logged.
 @return true on success, false on failure; page_zip will be left
 intact on failure, but page will be overwritten. */
ibool page_zip_reorganize(
    buf_block_t *block,  /*!< in/out: page with compressed page;
                         on the compressed page, in: size;
                         out: data, n_blobs,
                         m_start, m_end, m_nonempty */
    dict_index_t *index, /*!< in: index of the B-tree node */
    mtr_t *mtr);         /*!< in: mini-transaction */
