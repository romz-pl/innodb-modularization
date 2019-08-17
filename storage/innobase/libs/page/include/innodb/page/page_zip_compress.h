#pragma once

#include <innodb/univ/univ.h>

#include <innodb/disk/page_t.h>

struct page_zip_des_t;
struct dict_index_t;
struct mtr_t;

/** Compress a page.
 @return true on success, false on failure; page_zip will be left
 intact on failure. */
ibool page_zip_compress(page_zip_des_t *page_zip, /*!< in: size; out: data,
                                                  n_blobs, m_start, m_end,
                                                  m_nonempty */
                        const page_t *page,       /*!< in: uncompressed page */
                        dict_index_t *index,      /*!< in: index tree */
                        ulint level,              /*!< in: commpression level */
                        mtr_t *mtr);              /*!< in/out: mini-transaction,
                                                  or NULL */
