#include <innodb/page/page_zip_validate_low.h>


#ifdef UNIV_ZIP_DEBUG
/** Check that the compressed and decompressed pages match.
 @return true if valid, false if not */
ibool page_zip_validate_low(
    const page_zip_des_t *page_zip, /*!< in: compressed page */
    const page_t *page,             /*!< in: uncompressed page */
    const dict_index_t *index,      /*!< in: index of the page, if known */
    ibool sloppy)                   /*!< in: FALSE=strict,
                                    TRUE=ignore the MIN_REC_FLAG */
{
  page_zip_des_t temp_page_zip;
  byte *temp_page_buf;
  page_t *temp_page;
  ibool valid;

  if (memcmp(page_zip->data + FIL_PAGE_PREV, page + FIL_PAGE_PREV,
             FIL_PAGE_LSN - FIL_PAGE_PREV) ||
      memcmp(page_zip->data + FIL_PAGE_TYPE, page + FIL_PAGE_TYPE, 2) ||
      memcmp(page_zip->data + FIL_PAGE_DATA, page + FIL_PAGE_DATA,
             PAGE_DATA - FIL_PAGE_DATA)) {
    page_zip_fail(("page_zip_validate: page header\n"));
    page_zip_hexdump(page_zip, sizeof *page_zip);
    page_zip_hexdump(page_zip->data, page_zip_get_size(page_zip));
    page_zip_hexdump(page, UNIV_PAGE_SIZE);
    return (FALSE);
  }

  ut_a(page_is_comp(page));

  if (page_zip_validate_header_only) {
    return (TRUE);
  }

  /* page_zip_decompress() expects the uncompressed page to be
  UNIV_PAGE_SIZE aligned. */
  temp_page_buf = static_cast<byte *>(ut_malloc_nokey(2 * UNIV_PAGE_SIZE));
  temp_page = static_cast<byte *>(ut_align(temp_page_buf, UNIV_PAGE_SIZE));

  UNIV_MEM_ASSERT_RW(page, UNIV_PAGE_SIZE);
  UNIV_MEM_ASSERT_RW(page_zip->data, page_zip_get_size(page_zip));

  temp_page_zip = *page_zip;
  valid = page_zip_decompress_low(&temp_page_zip, temp_page, TRUE);
  if (!valid) {
    fputs("page_zip_validate(): failed to decompress\n", stderr);
    goto func_exit;
  }
  if (page_zip->n_blobs != temp_page_zip.n_blobs) {
    page_zip_fail(("page_zip_validate: n_blobs: %u!=%u\n", page_zip->n_blobs,
                   temp_page_zip.n_blobs));
    valid = FALSE;
  }
#ifdef UNIV_DEBUG
  if (page_zip->m_start != temp_page_zip.m_start) {
    page_zip_fail(("page_zip_validate: m_start: %u!=%u\n", page_zip->m_start,
                   temp_page_zip.m_start));
    valid = FALSE;
  }
#endif /* UNIV_DEBUG */
  if (page_zip->m_end != temp_page_zip.m_end) {
    page_zip_fail(("page_zip_validate: m_end: %u!=%u\n", page_zip->m_end,
                   temp_page_zip.m_end));
    valid = FALSE;
  }
  if (page_zip->m_nonempty != temp_page_zip.m_nonempty) {
    page_zip_fail(("page_zip_validate(): m_nonempty: %u!=%u\n",
                   page_zip->m_nonempty, temp_page_zip.m_nonempty));
    valid = FALSE;
  }
  if (memcmp(page + PAGE_HEADER, temp_page + PAGE_HEADER,
             UNIV_PAGE_SIZE - PAGE_HEADER - FIL_PAGE_DATA_END)) {
    /* In crash recovery, the "minimum record" flag may be
    set incorrectly until the mini-transaction is
    committed.  Let us tolerate that difference when we
    are performing a sloppy validation. */

    ulint *offsets;
    mem_heap_t *heap;
    const rec_t *rec;
    const rec_t *trec;
    byte info_bits_diff;
    ulint offset = rec_get_next_offs(page + PAGE_NEW_INFIMUM, TRUE);
    ut_a(offset >= PAGE_NEW_SUPREMUM);
    offset -= 5 /*REC_NEW_INFO_BITS*/;

    info_bits_diff = page[offset] ^ temp_page[offset];

    if (info_bits_diff == REC_INFO_MIN_REC_FLAG) {
      temp_page[offset] = page[offset];

      if (!memcmp(page + PAGE_HEADER, temp_page + PAGE_HEADER,
                  UNIV_PAGE_SIZE - PAGE_HEADER - FIL_PAGE_DATA_END)) {
        /* Only the minimum record flag
        differed.  Let us ignore it. */
        page_zip_fail(
            ("page_zip_validate:"
             " min_rec_flag"
             " (%s%lu,%lu,0x%02lx)\n",
             sloppy ? "ignored, " : "", page_get_space_id(page),
             page_get_page_no(page), (ulong)page[offset]));
        /* We don't check for spatial index, since
        the "minimum record" could be deleted when
        doing rtr_update_mbr_field.
        GIS_FIXME: need to validate why
        rtr_update_mbr_field.() could affect this */
        if (index && dict_index_is_spatial(index)) {
          valid = true;
        } else {
          valid = sloppy;
        }
        goto func_exit;
      }
    }

    /* Compare the pointers in the PAGE_FREE list. */
    rec = page_header_get_ptr(page, PAGE_FREE);
    trec = page_header_get_ptr(temp_page, PAGE_FREE);

    while (rec || trec) {
      if (page_offset(rec) != page_offset(trec)) {
        page_zip_fail(
            ("page_zip_validate:"
             " PAGE_FREE list: %u!=%u\n",
             (unsigned)page_offset(rec), (unsigned)page_offset(trec)));
        valid = FALSE;
        goto func_exit;
      }

      rec = page_rec_get_next_low(rec, TRUE);
      trec = page_rec_get_next_low(trec, TRUE);
    }

    /* Compare the records. */
    heap = NULL;
    offsets = NULL;
    rec = page_rec_get_next_low(page + PAGE_NEW_INFIMUM, TRUE);
    trec = page_rec_get_next_low(temp_page + PAGE_NEW_INFIMUM, TRUE);

    do {
      if (page_offset(rec) != page_offset(trec)) {
        page_zip_fail(
            ("page_zip_validate:"
             " record list: 0x%02x!=0x%02x\n",
             (unsigned)page_offset(rec), (unsigned)page_offset(trec)));
        valid = FALSE;
        break;
      }

      if (index) {
        /* Compare the data. */
        offsets = rec_get_offsets(rec, index, offsets, ULINT_UNDEFINED, &heap);

        if (memcmp(rec - rec_offs_extra_size(offsets),
                   trec - rec_offs_extra_size(offsets),
                   rec_offs_size(offsets))) {
          page_zip_fail(
              ("page_zip_validate:"
               " record content: 0x%02x",
               (unsigned)page_offset(rec)));
          valid = FALSE;
          break;
        }
      }

      rec = page_rec_get_next_low(rec, TRUE);
      trec = page_rec_get_next_low(trec, TRUE);
    } while (rec || trec);

    if (heap) {
      mem_heap_free(heap);
    }
  }

func_exit:
  if (!valid) {
    page_zip_hexdump(page_zip, sizeof *page_zip);
    page_zip_hexdump(page_zip->data, page_zip_get_size(page_zip));
    page_zip_hexdump(page, UNIV_PAGE_SIZE);
    page_zip_hexdump(temp_page, UNIV_PAGE_SIZE);
  }
  ut_free(temp_page_buf);
  return (valid);
}



#endif
