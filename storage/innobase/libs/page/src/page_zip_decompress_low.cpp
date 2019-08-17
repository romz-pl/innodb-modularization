#include <innodb/page/page_zip_decompress_low.h>

#include <innodb/dict_mem/dict_index_get_n_fields.h>
#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/disk/fil_page_get_type.h>
#include <innodb/disk/flags.h>
#include <innodb/disk/page_type_t.h>
#include <innodb/machine/data.h>
#include <innodb/memory/mem_heap_alloc.h>
#include <innodb/memory/mem_heap_create.h>
#include <innodb/memory/mem_heap_free.h>
#include <innodb/memory_check/memory_check.h>
#include <innodb/page/flag.h>
#include <innodb/page/header.h>
#include <innodb/page/infimum_data.h>
#include <innodb/page/infimum_extra.h>
#include <innodb/page/page_dir_get_n_heap.h>
#include <innodb/page/page_is_comp.h>
#include <innodb/page/page_is_empty.h>
#include <innodb/page/page_is_leaf.h>
#include <innodb/page/page_zip_decompress_clust.h>
#include <innodb/page/page_zip_decompress_node_ptrs.h>
#include <innodb/page/page_zip_decompress_sec.h>
#include <innodb/page/page_zip_des_t.h>
#include <innodb/page/page_zip_dir_decode.h>
#include <innodb/page/page_zip_dir_get.h>
#include <innodb/page/page_zip_fail.h>
#include <innodb/page/page_zip_fields_decode.h>
#include <innodb/page/page_zip_fields_free.h>
#include <innodb/page/page_zip_get_size.h>
#include <innodb/page/page_zip_set_alloc.h>
#include <innodb/page/page_zip_set_extra_bytes.h>
#include <innodb/page/supremum_extra_data.h>
#include <innodb/record/rec_set_next_offs_new.h>
#include <innodb/univ/rec_t.h>


#include <zlib.h>


/** Decompress a page.  This function should tolerate errors on the compressed
 page.  Instead of letting assertions fail, it will return FALSE if an
 inconsistency is detected.
 @return true on success, false on failure */
ibool page_zip_decompress_low(
    page_zip_des_t *page_zip, /*!< in: data, ssize;
                             out: m_start, m_end, m_nonempty, n_blobs */
    page_t *page,             /*!< out: uncompressed page, may be trashed */
    ibool all)                /*!< in: TRUE=decompress the whole page;
                              FALSE=verify but do not copy some
                              page header fields that should not change
                              after page creation */
{
  z_stream d_stream;
  dict_index_t *index = NULL;
  rec_t **recs;  /*!< dense page directory, sorted by address */
  ulint n_dense; /* number of user records on the page */
  ulint trx_id_col = ULINT_UNDEFINED;
  mem_heap_t *heap;
  ulint *offsets;

  ut_ad(page_zip_simple_validate(page_zip));
  UNIV_MEM_ASSERT_W(page, UNIV_PAGE_SIZE);
  UNIV_MEM_ASSERT_RW(page_zip->data, page_zip_get_size(page_zip));

  /* The dense directory excludes the infimum and supremum records. */
  n_dense = page_dir_get_n_heap(page_zip->data) - PAGE_HEAP_NO_USER_LOW;
  if (UNIV_UNLIKELY(n_dense * PAGE_ZIP_DIR_SLOT_SIZE >=
                    page_zip_get_size(page_zip))) {
    page_zip_fail(("page_zip_decompress 1: %lu %lu\n", (ulong)n_dense,
                   (ulong)page_zip_get_size(page_zip)));
    return (FALSE);
  }

  heap = mem_heap_create(n_dense * (3 * sizeof *recs) + UNIV_PAGE_SIZE);

  recs = static_cast<rec_t **>(mem_heap_alloc(heap, n_dense * sizeof *recs));

  if (all) {
    /* Copy the page header. */
    memcpy(page, page_zip->data, PAGE_DATA);
  } else {
  /* Check that the bytes that we skip are identical. */
#if defined UNIV_DEBUG || defined UNIV_ZIP_DEBUG
    ut_a(!memcmp(FIL_PAGE_TYPE + page, FIL_PAGE_TYPE + page_zip->data,
                 PAGE_HEADER - FIL_PAGE_TYPE));
    ut_a(!memcmp(PAGE_HEADER + PAGE_LEVEL + page,
                 PAGE_HEADER + PAGE_LEVEL + page_zip->data,
                 PAGE_DATA - (PAGE_HEADER + PAGE_LEVEL)));
#endif /* UNIV_DEBUG || UNIV_ZIP_DEBUG */

    /* Copy the mutable parts of the page header. */
    memcpy(page, page_zip->data, FIL_PAGE_TYPE);
    memcpy(PAGE_HEADER + page, PAGE_HEADER + page_zip->data,
           PAGE_LEVEL - PAGE_N_DIR_SLOTS);

#if defined UNIV_DEBUG || defined UNIV_ZIP_DEBUG
    /* Check that the page headers match after copying. */
    ut_a(!memcmp(page, page_zip->data, PAGE_DATA));
#endif /* UNIV_DEBUG || UNIV_ZIP_DEBUG */
  }

#ifdef UNIV_ZIP_DEBUG
  /* Clear the uncompressed page, except the header. */
  memset(PAGE_DATA + page, 0x55, UNIV_PAGE_SIZE - PAGE_DATA);
#endif /* UNIV_ZIP_DEBUG */
  UNIV_MEM_INVALID(PAGE_DATA + page, UNIV_PAGE_SIZE - PAGE_DATA);

  /* Copy the page directory. */
  if (UNIV_UNLIKELY(!page_zip_dir_decode(page_zip, page, recs, n_dense))) {
  zlib_error:
    mem_heap_free(heap);
    return (FALSE);
  }

  /* Copy the infimum and supremum records. */
  memcpy(page + (PAGE_NEW_INFIMUM - REC_N_NEW_EXTRA_BYTES), infimum_extra,
         sizeof infimum_extra);
  if (page_is_empty(page)) {
    rec_set_next_offs_new(page + PAGE_NEW_INFIMUM, PAGE_NEW_SUPREMUM);
  } else {
    rec_set_next_offs_new(
        page + PAGE_NEW_INFIMUM,
        page_zip_dir_get(page_zip, 0) & PAGE_ZIP_DIR_SLOT_MASK);
  }
  memcpy(page + PAGE_NEW_INFIMUM, infimum_data, sizeof infimum_data);
  memcpy(page + (PAGE_NEW_SUPREMUM - REC_N_NEW_EXTRA_BYTES + 1),
         supremum_extra_data, sizeof supremum_extra_data);

  page_zip_set_alloc(&d_stream, heap);

  d_stream.next_in = page_zip->data + PAGE_DATA;
  /* Subtract the space reserved for
  the page header and the end marker of the modification log. */
  d_stream.avail_in =
      static_cast<uInt>(page_zip_get_size(page_zip) - (PAGE_DATA + 1));
  d_stream.next_out = page + PAGE_ZIP_START;
  d_stream.avail_out = UNIV_PAGE_SIZE - PAGE_ZIP_START;

  if (UNIV_UNLIKELY(inflateInit2(&d_stream, UNIV_PAGE_SIZE_SHIFT) != Z_OK)) {
    ut_error;
  }

  /* Decode the zlib header and the index information. */
  if (UNIV_UNLIKELY(inflate(&d_stream, Z_BLOCK) != Z_OK)) {
    page_zip_fail(
        ("page_zip_decompress:"
         " 1 inflate(Z_BLOCK)=%s\n",
         d_stream.msg));
    goto zlib_error;
  }

  if (UNIV_UNLIKELY(inflate(&d_stream, Z_BLOCK) != Z_OK)) {
    page_zip_fail(
        ("page_zip_decompress:"
         " 2 inflate(Z_BLOCK)=%s\n",
         d_stream.msg));
    goto zlib_error;
  }

  index = page_zip_fields_decode(page + PAGE_ZIP_START, d_stream.next_out,
                                 page_is_leaf(page) ? &trx_id_col : NULL,
                                 fil_page_get_type(page) == FIL_PAGE_RTREE);

  if (UNIV_UNLIKELY(!index)) {
    goto zlib_error;
  }

  /* Decompress the user records. */
  page_zip->n_blobs = 0;
  d_stream.next_out = page + PAGE_ZIP_START;

  {
    /* Pre-allocate the offsets for rec_get_offsets_reverse(). */
    ulint n = 1 + 1 /* node ptr */ + REC_OFFS_HEADER_SIZE +
              dict_index_get_n_fields(index);

    offsets = static_cast<ulint *>(mem_heap_alloc(heap, n * sizeof(ulint)));

    *offsets = n;
  }

  /* Decompress the records in heap_no order. */
  if (!page_is_leaf(page)) {
    /* This is a node pointer page. */
    ulint info_bits;

    if (UNIV_UNLIKELY(!page_zip_decompress_node_ptrs(
            page_zip, &d_stream, recs, n_dense, index, offsets, heap))) {
      goto err_exit;
    }

    info_bits = mach_read_from_4(page + FIL_PAGE_PREV) == FIL_NULL
                    ? REC_INFO_MIN_REC_FLAG
                    : 0;

    if (UNIV_UNLIKELY(!page_zip_set_extra_bytes(page_zip, page, info_bits))) {
      goto err_exit;
    }
  } else if (UNIV_LIKELY(trx_id_col == ULINT_UNDEFINED)) {
    /* This is a leaf page in a secondary index. */
    if (UNIV_UNLIKELY(!page_zip_decompress_sec(page_zip, &d_stream, recs,
                                               n_dense, index, offsets))) {
      goto err_exit;
    }

    if (UNIV_UNLIKELY(!page_zip_set_extra_bytes(page_zip, page, 0))) {
    err_exit:
      page_zip_fields_free(index);
      mem_heap_free(heap);
      return (FALSE);
    }
  } else {
    /* This is a leaf page in a clustered index. */
    if (UNIV_UNLIKELY(!page_zip_decompress_clust(page_zip, &d_stream, recs,
                                                 n_dense, index, trx_id_col,
                                                 offsets, heap))) {
      goto err_exit;
    }

    if (UNIV_UNLIKELY(!page_zip_set_extra_bytes(page_zip, page, 0))) {
      goto err_exit;
    }
  }

  ut_a(page_is_comp(page));
  UNIV_MEM_ASSERT_RW(page, UNIV_PAGE_SIZE);

  page_zip_fields_free(index);
  mem_heap_free(heap);

  return (TRUE);
}
