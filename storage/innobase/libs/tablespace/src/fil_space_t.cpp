#include <innodb/tablespace/fil_space_t.h>

#include <innodb/sync_rw/rw_lock_own.h>

/** System tablespace. */
fil_space_t *fil_space_t::s_sys_space;

/** Redo log tablespace */
fil_space_t *fil_space_t::s_redo_space;


#ifndef UNIV_HOTBACKUP
#ifdef UNIV_DEBUG

/** Print the extent descriptor pages of this tablespace into
the given file.
@param[in]	filename	the output file name. */
void fil_space_t::print_xdes_pages(const char *filename) const {
  std::ofstream out(filename);
  print_xdes_pages(out);
}

/** Print the extent descriptor pages of this tablespace into
the given file.
@param[in]	out	the output file name.
@return	the output stream. */
std::ostream &fil_space_t::print_xdes_pages(std::ostream &out) const {
  mtr_t mtr;
  const page_size_t page_size(flags);

  mtr_start(&mtr);

  for (page_no_t i = 0; i < 100; ++i) {
    page_no_t xdes_page_no = i * UNIV_PAGE_SIZE;

    if (xdes_page_no >= size) {
      break;
    }

    buf_block_t *xdes_block =
        buf_page_get(page_id_t(id, xdes_page_no), page_size, RW_S_LATCH, &mtr);

    page_t *page = buf_block_get_frame(xdes_block);

    ulint page_type = fil_page_get_type(page);

    switch (page_type) {
      case FIL_PAGE_TYPE_ALLOCATED:

        ut_ad(xdes_page_no >= free_limit);

        mtr_commit(&mtr);
        return (out);

      case FIL_PAGE_TYPE_FSP_HDR:
      case FIL_PAGE_TYPE_XDES:
        break;
      default:
        ut_error;
    }

    xdes_page_print(out, page, xdes_page_no, &mtr);
  }

  mtr_commit(&mtr);
  return (out);
}
#endif /* UNIV_DEBUG */
#endif


/** Release the reserved free extents.
@param[in]	n_reserved	number of reserved extents */
void fil_space_t::release_free_extents(ulint n_reserved) {
#ifndef UNIV_HOTBACKUP
  ut_ad(rw_lock_own(&latch, RW_LOCK_X));
#endif /* !UNIV_HOTBACKUP */

  ut_a(n_reserved < std::numeric_limits<uint32_t>::max());
  ut_a(n_reserved_extents >= n_reserved);

  n_reserved_extents -= (uint32_t)n_reserved;
}
