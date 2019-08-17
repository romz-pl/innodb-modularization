/*****************************************************************************

Copyright (c) 2005, 2018, Oracle and/or its affiliates. All Rights Reserved.
Copyright (c) 2012, Facebook Inc.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License, version 2.0, as published by the
Free Software Foundation.

This program is also distributed with certain software (including but not
limited to OpenSSL) that is licensed under separate terms, as designated in a
particular file or component or in included license documentation. The authors
of MySQL hereby grant you an additional permission to link the program and
your derivative works with the separately licensed software that they have
included with MySQL.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU General Public License, version 2.0,
for more details.

You should have received a copy of the GNU General Public License along with
this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA

*****************************************************************************/

/** @file page/page0zip.cc
 Compressed page interface

 Created June 2005 by Marko Makela
 *******************************************************/

#include <innodb/record/rec_get_n_extern_new.h>
#include <innodb/record/rec_get_converted_size_comp_prefix.h>
#include <innodb/record/rec_offs_any_extern.h>
#include <innodb/record/rec_offs_nth_extern.h>
#include <innodb/memory/mem_heap_zalloc.h>
#include <innodb/assert/ASSERT_ZERO.h>
#include <innodb/assert/ASSERT_ZERO_BLOB.h>
#include <innodb/buf_frame/buf_frame_copy.h>
#include <innodb/disk/page_size_t.h>
#include <innodb/page/page_zip_dir_encode.h>
#include <innodb/page/page_zip_dir_find.h>
#include <innodb/page/page_zip_dir_start.h>
#include <innodb/page/page_zip_dir_start_low.h>
#include <innodb/page/page_zip_fixed_field_encode.h>
#include <innodb/page/page_zip_get_size.h>
#include <innodb/record/rec_offs_data_size.h>
#include <innodb/record/rec_offs_extra_size.h>
#include <innodb/record/rec_offs_n_extern.h>
#include <innodb/time/ut_time_us.h>
#include <innodb/page/page_zip_dir_get.h>
#include <innodb/page/page_zip_des_t.h>

#include "page0zip.h"


/** A BLOB field reference full of zero, for use in assertions and tests.
Initially, BLOB field references are set to zero, in
dtuple_convert_big_rec(). */
const byte field_ref_zero[FIELD_REF_SIZE] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};

#include <zlib.h>

#include "btr0cur.h"
#include "dict0dict.h"
#include "log0recv.h"
#include "mtr0log.h"
#include "page0page.h"
#ifndef UNIV_HOTBACKUP
#include "btr0sea.h"
#include "buf0buf.h"
#include "buf0lru.h"
#include "dict0boot.h"
#include "lock0lock.h"
#include "srv0mon.h"
#include "srv0srv.h"
#endif /* !UNIV_HOTBACKUP */
#include <innodb/crc32/crc32.h>
#ifdef UNIV_HOTBACKUP

#define lock_move_reorganize_page(block, temp_block) ((void)0)
#define buf_LRU_stat_inc_unzip() ((void)0)
#define MONITOR_INC(x) ((void)0)
#endif /* !UNIV_HOTBACKUP */

#include <algorithm>
#include <map>
#include <innodb/page/page_zip_stat.h>

static_assert(DATA_TRX_ID_LEN == 6, "DATA_TRX_ID_LEN != 6");
static_assert(DATA_ROLL_PTR_LEN == 7, "DATA_ROLL_PTR_LEN != 7");
static_assert(DATA_TRX_ID + 1 == DATA_ROLL_PTR, "DATA_TRX_ID invalid value!");





/* Please refer to ../include/page0zip.ic for a description of the
compressed page format. */










#if 0 || defined UNIV_DEBUG || defined UNIV_ZIP_DEBUG
/** Symbol for enabling compression and decompression diagnostics */
#define PAGE_ZIP_COMPRESS_DBG
#endif

#ifdef PAGE_ZIP_COMPRESS_DBG
/** Set this variable in a debugger to enable
excessive logging in page_zip_compress(). */
static ibool page_zip_compress_dbg;
/** Set this variable in a debugger to enable
binary logging of the data passed to deflate().
When this variable is nonzero, it will act
as a log file name generator. */
static unsigned page_zip_compress_log;

/** Wrapper for deflate().  Log the operation if page_zip_compress_dbg is set.
 @return deflate() status: Z_OK, Z_BUF_ERROR, ... */
static int page_zip_compress_deflate(
    FILE *logfile,  /*!< in: log file, or NULL */
    z_streamp strm, /*!< in/out: compressed stream for deflate() */
    int flush)      /*!< in: deflate() flushing method */
{
  int status;
  if (UNIV_UNLIKELY(page_zip_compress_dbg)) {
    ut_print_buf(stderr, strm->next_in, strm->avail_in);
  }
  if (UNIV_LIKELY_NULL(logfile)) {
    if (fwrite(strm->next_in, 1, strm->avail_in, logfile) != strm->avail_in) {
      perror("fwrite");
    }
  }
  status = deflate(strm, flush);
  if (UNIV_UNLIKELY(page_zip_compress_dbg)) {
    fprintf(stderr, " -> %d\n", status);
  }
  return (status);
}

/* Redefine deflate(). */
#undef deflate
/** Debug wrapper for the zlib compression routine deflate().
Log the operation if page_zip_compress_dbg is set.
@param strm in/out: compressed stream
@param flush in: flushing method
@return deflate() status: Z_OK, Z_BUF_ERROR, ... */
#define deflate(strm, flush) page_zip_compress_deflate(logfile, strm, flush)


#else /* PAGE_ZIP_COMPRESS_DBG */


#endif /* PAGE_ZIP_COMPRESS_DBG */


#include <innodb/page/LOGFILE.h>
#include <innodb/page/FILE_LOGFILE.h>







#ifdef UNIV_ZIP_DEBUG
/** Dump a block of memory on the standard error stream. */
static void page_zip_hexdump_func(
    const char *name, /*!< in: name of the data structure */
    const void *buf,  /*!< in: data */
    ulint size)       /*!< in: length of the data, in bytes */
{
  const byte *s = static_cast<const byte *>(buf);
  ulint addr;
  const ulint width = 32; /* bytes per line */

  fprintf(stderr, "%s:\n", name);

  for (addr = 0; addr < size; addr += width) {
    ulint i;

    fprintf(stderr, "%04lx ", (ulong)addr);

    i = std::min(width, size - addr);

    while (i--) {
      fprintf(stderr, "%02x", *s++);
    }

    putc('\n', stderr);
  }
}

/** Dump a block of memory on the standard error stream.
@param buf in: data
@param size in: length of the data, in bytes */
#define page_zip_hexdump(buf, size) page_zip_hexdump_func(#buf, buf, size)

/** Flag: make page_zip_validate() compare page headers only */
ibool page_zip_validate_header_only = FALSE;

#define page_zip_fail(fmt_args) page_zip_fail_func fmt_args
int page_zip_fail_func(const char *fmt, ...);

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


#endif /* UNIV_ZIP_DEBUG */

#ifdef UNIV_DEBUG
/** Assert that the compressed and decompressed page headers match.
 @return true */
static ibool page_zip_header_cmp(
    const page_zip_des_t *page_zip, /*!< in: compressed page */
    const byte *page)               /*!< in: uncompressed page */
{
  ut_ad(!memcmp(page_zip->data + FIL_PAGE_PREV, page + FIL_PAGE_PREV,
                FIL_PAGE_LSN - FIL_PAGE_PREV));
  ut_ad(!memcmp(page_zip->data + FIL_PAGE_TYPE, page + FIL_PAGE_TYPE, 2));
  ut_ad(!memcmp(page_zip->data + FIL_PAGE_DATA, page + FIL_PAGE_DATA,
                PAGE_DATA - FIL_PAGE_DATA));

  return (TRUE);
}
#endif /* UNIV_DEBUG */

/** Write a record on the compressed page that contains externally stored
 columns.  The data must already have been written to the uncompressed page.
 @return end of modification log */
static byte *page_zip_write_rec_ext(
    page_zip_des_t *page_zip,  /*!< in/out: compressed page */
    const page_t *page,        /*!< in: page containing rec */
    const byte *rec,           /*!< in: record being written */
    const dict_index_t *index, /*!< in: record descriptor */
    const ulint *offsets,      /*!< in: rec_get_offsets(rec, index) */
    ulint create,              /*!< in: nonzero=insert, zero=update */
    ulint trx_id_col,          /*!< in: position of DB_TRX_ID */
    ulint heap_no,             /*!< in: heap number of rec */
    byte *storage,             /*!< in: end of dense page directory */
    byte *data)                /*!< in: end of modification log */
{
  const byte *start = rec;
  ulint i;
  ulint len;
  byte *externs = storage;
  ulint n_ext = rec_offs_n_extern(offsets);

  ut_ad(rec_offs_validate(rec, index, offsets));
  UNIV_MEM_ASSERT_RW(rec, rec_offs_data_size(offsets));
  UNIV_MEM_ASSERT_RW(rec - rec_offs_extra_size(offsets),
                     rec_offs_extra_size(offsets));

  externs -= (DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN) *
             (page_dir_get_n_heap(page) - PAGE_HEAP_NO_USER_LOW);

  /* Note that this will not take into account
  the BLOB columns of rec if create==TRUE. */
  ut_ad(data + rec_offs_data_size(offsets) -
            (DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN) -
            n_ext * BTR_EXTERN_FIELD_REF_SIZE <
        externs - BTR_EXTERN_FIELD_REF_SIZE * page_zip->n_blobs);

  {
    ulint blob_no = page_zip_get_n_prev_extern(page_zip, rec, index);
    byte *ext_end = externs - page_zip->n_blobs * BTR_EXTERN_FIELD_REF_SIZE;
    ut_ad(blob_no <= page_zip->n_blobs);
    externs -= blob_no * BTR_EXTERN_FIELD_REF_SIZE;

    if (create) {
      page_zip->n_blobs += static_cast<unsigned>(n_ext);
      ASSERT_ZERO_BLOB(ext_end - n_ext * BTR_EXTERN_FIELD_REF_SIZE);
      memmove(ext_end - n_ext * BTR_EXTERN_FIELD_REF_SIZE, ext_end,
              externs - ext_end);
    }

    ut_a(blob_no + n_ext <= page_zip->n_blobs);
  }

  for (i = 0; i < rec_offs_n_fields(offsets); i++) {
    const byte *src;

    if (UNIV_UNLIKELY(i == trx_id_col)) {
      ut_ad(!rec_offs_nth_extern(offsets, i));
      ut_ad(!rec_offs_nth_extern(offsets, i + 1));
      /* Locate trx_id and roll_ptr. */
      src = rec_get_nth_field(rec, offsets, i, &len);
      ut_ad(len == DATA_TRX_ID_LEN);
      ut_ad(src + DATA_TRX_ID_LEN ==
            rec_get_nth_field(rec, offsets, i + 1, &len));
      ut_ad(len == DATA_ROLL_PTR_LEN);

      /* Log the preceding fields. */
      ASSERT_ZERO(data, src - start);
      memcpy(data, start, src - start);
      data += src - start;
      start = src + (DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN);

      /* Store trx_id and roll_ptr. */
      memcpy(storage - (DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN) * (heap_no - 1),
             src, DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN);
      i++; /* skip also roll_ptr */
    } else if (rec_offs_nth_extern(offsets, i)) {
      src = rec_get_nth_field(rec, offsets, i, &len);

      ut_ad(index->is_clustered());
      ut_ad(len >= BTR_EXTERN_FIELD_REF_SIZE);
      src += len - BTR_EXTERN_FIELD_REF_SIZE;

      ASSERT_ZERO(data, src - start);
      memcpy(data, start, src - start);
      data += src - start;
      start = src + BTR_EXTERN_FIELD_REF_SIZE;

      /* Store the BLOB pointer. */
      externs -= BTR_EXTERN_FIELD_REF_SIZE;
      ut_ad(data < externs);
      memcpy(externs, src, BTR_EXTERN_FIELD_REF_SIZE);
    }
  }

  /* Log the last bytes of the record. */
  len = rec_offs_data_size(offsets) - (start - rec);

  ASSERT_ZERO(data, len);
  memcpy(data, start, len);
  data += len;

  return (data);
}

/** Write an entire record on the compressed page.  The data must already
 have been written to the uncompressed page. */
void page_zip_write_rec(
    page_zip_des_t *page_zip,  /*!< in/out: compressed page */
    const byte *rec,           /*!< in: record being written */
    const dict_index_t *index, /*!< in: the index the record belongs to */
    const ulint *offsets,      /*!< in: rec_get_offsets(rec, index) */
    ulint create)              /*!< in: nonzero=insert, zero=update */
{
  const page_t *page;
  byte *data;
  byte *storage;
  ulint heap_no;
  byte *slot;

  ut_ad(page_zip_simple_validate(page_zip));
  ut_ad(page_zip_get_size(page_zip) > PAGE_DATA + page_zip_dir_size(page_zip));
  ut_ad(rec_offs_comp(offsets));
  ut_ad(rec_offs_validate(rec, index, offsets));

  ut_ad(page_zip->m_start >= PAGE_DATA);

  page = page_align(rec);

  ut_ad(page_zip_header_cmp(page_zip, page));
  ut_ad(page_simple_validate_new((page_t *)page));

  UNIV_MEM_ASSERT_RW(page_zip->data, page_zip_get_size(page_zip));
  UNIV_MEM_ASSERT_RW(rec, rec_offs_data_size(offsets));
  UNIV_MEM_ASSERT_RW(rec - rec_offs_extra_size(offsets),
                     rec_offs_extra_size(offsets));

  slot = page_zip_dir_find(page_zip, page_offset(rec));
  ut_a(slot);
  /* Copy the delete mark. */
  if (rec_get_deleted_flag(rec, TRUE)) {
    *slot |= PAGE_ZIP_DIR_SLOT_DEL >> 8;
  } else {
    *slot &= ~(PAGE_ZIP_DIR_SLOT_DEL >> 8);
  }

  ut_ad(rec_get_start((rec_t *)rec, offsets) >= page + PAGE_ZIP_START);
  ut_ad(rec_get_end((rec_t *)rec, offsets) <=
        page + UNIV_PAGE_SIZE - PAGE_DIR -
            PAGE_DIR_SLOT_SIZE * page_dir_get_n_slots(page));

  heap_no = rec_get_heap_no_new(rec);
  ut_ad(heap_no >= PAGE_HEAP_NO_USER_LOW); /* not infimum or supremum */
  ut_ad(heap_no < page_dir_get_n_heap(page));

  /* Append to the modification log. */
  data = page_zip->data + page_zip->m_end;
  ut_ad(!*data);

  /* Identify the record by writing its heap number - 1.
  0 is reserved to indicate the end of the modification log. */

  if (UNIV_UNLIKELY(heap_no - 1 >= 64)) {
    *data++ = (byte)(0x80 | (heap_no - 1) >> 7);
    ut_ad(!*data);
  }
  *data++ = (byte)((heap_no - 1) << 1);
  ut_ad(!*data);

  {
    const byte *start = rec - rec_offs_extra_size(offsets);
    const byte *b = rec - REC_N_NEW_EXTRA_BYTES;

    /* Write the extra bytes backwards, so that
    rec_offs_extra_size() can be easily computed in
    page_zip_apply_log() by invoking
    rec_get_offsets_reverse(). */

    while (b != start) {
      *data++ = *--b;
      ut_ad(!*data);
    }
  }

  /* Write the data bytes.  Store the uncompressed bytes separately. */
  storage = page_zip_dir_start(page_zip);

  if (page_is_leaf(page)) {
    ulint len;

    if (index->is_clustered()) {
      ulint trx_id_col;

      trx_id_col = index->get_sys_col_pos(DATA_TRX_ID);
      ut_ad(trx_id_col != ULINT_UNDEFINED);

      /* Store separately trx_id, roll_ptr and
      the BTR_EXTERN_FIELD_REF of each BLOB column. */
      if (rec_offs_any_extern(offsets)) {
        data =
            page_zip_write_rec_ext(page_zip, page, rec, index, offsets, create,
                                   trx_id_col, heap_no, storage, data);
      } else {
        /* Locate trx_id and roll_ptr. */
        const byte *src = rec_get_nth_field(rec, offsets, trx_id_col, &len);
        ut_ad(len == DATA_TRX_ID_LEN);
        ut_ad(src + DATA_TRX_ID_LEN ==
              rec_get_nth_field(rec, offsets, trx_id_col + 1, &len));
        ut_ad(len == DATA_ROLL_PTR_LEN);

        /* Log the preceding fields. */
        ASSERT_ZERO(data, src - rec);
        memcpy(data, rec, src - rec);
        data += src - rec;

        /* Store trx_id and roll_ptr. */
        memcpy(storage - (DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN) * (heap_no - 1),
               src, DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN);

        src += DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN;

        /* Log the last bytes of the record. */
        len = rec_offs_data_size(offsets) - (src - rec);

        ASSERT_ZERO(data, len);
        memcpy(data, src, len);
        data += len;
      }
    } else {
      /* Leaf page of a secondary index:
      no externally stored columns */
      ut_ad(index->get_sys_col_pos(DATA_TRX_ID) == ULINT_UNDEFINED);
      ut_ad(!rec_offs_any_extern(offsets));

      /* Log the entire record. */
      len = rec_offs_data_size(offsets);

      ASSERT_ZERO(data, len);
      memcpy(data, rec, len);
      data += len;
    }
  } else {
    /* This is a node pointer page. */
    ulint len;

    /* Non-leaf nodes should not have any externally
    stored columns. */
    ut_ad(!rec_offs_any_extern(offsets));

    /* Copy the data bytes, except node_ptr. */
    len = rec_offs_data_size(offsets) - REC_NODE_PTR_SIZE;
    ut_ad(data + len <
          storage - REC_NODE_PTR_SIZE *
                        (page_dir_get_n_heap(page) - PAGE_HEAP_NO_USER_LOW));
    ASSERT_ZERO(data, len);
    memcpy(data, rec, len);
    data += len;

    /* Copy the node pointer to the uncompressed area. */
    memcpy(storage - REC_NODE_PTR_SIZE * (heap_no - 1), rec + len,
           REC_NODE_PTR_SIZE);
  }

  ut_a(!*data);
  ut_ad((ulint)(data - page_zip->data) < page_zip_get_size(page_zip));
  page_zip->m_end = data - page_zip->data;
  page_zip->m_nonempty = TRUE;

#ifdef UNIV_ZIP_DEBUG
  ut_a(page_zip_validate(page_zip, page_align(rec), index));
#endif /* UNIV_ZIP_DEBUG */
}

/** Parses a log record of writing a BLOB pointer of a record.
 @return end of log record or NULL */
byte *page_zip_parse_write_blob_ptr(
    byte *ptr,                /*!< in: redo log buffer */
    byte *end_ptr,            /*!< in: redo log buffer end */
    page_t *page,             /*!< in/out: uncompressed page */
    page_zip_des_t *page_zip) /*!< in/out: compressed page */
{
  ulint offset;
  ulint z_offset;

  ut_ad(ptr != NULL);
  ut_ad(end_ptr != NULL);
  ut_ad(!page == !page_zip);

  if (UNIV_UNLIKELY(end_ptr < ptr + (2 + 2 + BTR_EXTERN_FIELD_REF_SIZE))) {
    return (NULL);
  }

  offset = mach_read_from_2(ptr);
  z_offset = mach_read_from_2(ptr + 2);

  if (offset < PAGE_ZIP_START || offset >= UNIV_PAGE_SIZE ||
      z_offset >= UNIV_PAGE_SIZE) {
  corrupt:
    recv_sys->found_corrupt_log = TRUE;

    return (NULL);
  }

  if (page) {
    if (!page_zip || !page_is_leaf(page)) {
      goto corrupt;
    }

#ifdef UNIV_ZIP_DEBUG
    ut_a(page_zip_validate(page_zip, page, NULL));
#endif /* UNIV_ZIP_DEBUG */

    memcpy(page + offset, ptr + 4, BTR_EXTERN_FIELD_REF_SIZE);
    memcpy(page_zip->data + z_offset, ptr + 4, BTR_EXTERN_FIELD_REF_SIZE);

#ifdef UNIV_ZIP_DEBUG
    ut_a(page_zip_validate(page_zip, page, NULL));
#endif /* UNIV_ZIP_DEBUG */
  }

  return (ptr + (2 + 2 + BTR_EXTERN_FIELD_REF_SIZE));
}

/** Write a BLOB pointer of a record on the leaf page of a clustered index.
 The information must already have been updated on the uncompressed page. */
void page_zip_write_blob_ptr(
    page_zip_des_t *page_zip,  /*!< in/out: compressed page */
    const byte *rec,           /*!< in/out: record whose data is being
                               written */
    const dict_index_t *index, /*!< in: index of the page */
    const ulint *offsets,      /*!< in: rec_get_offsets(rec, index) */
    ulint n,                   /*!< in: column index */
    mtr_t *mtr)                /*!< in: mini-transaction handle,
                       or NULL if no logging is needed */
{
  const byte *field;
  byte *externs;
  const page_t *page = page_align(rec);
  ulint blob_no;
  ulint len;

  ut_ad(page_zip != NULL);
  ut_ad(rec != NULL);
  ut_ad(index != NULL);
  ut_ad(offsets != NULL);
  ut_ad(page_simple_validate_new((page_t *)page));
  ut_ad(page_zip_simple_validate(page_zip));
  ut_ad(page_zip_get_size(page_zip) > PAGE_DATA + page_zip_dir_size(page_zip));
  ut_ad(rec_offs_comp(offsets));
  ut_ad(rec_offs_validate(rec, NULL, offsets));
  ut_ad(rec_offs_any_extern(offsets));
  ut_ad(rec_offs_nth_extern(offsets, n));

  ut_ad(page_zip->m_start >= PAGE_DATA);
  ut_ad(page_zip_header_cmp(page_zip, page));

  ut_ad(page_is_leaf(page));
  ut_ad(index->is_clustered());

  UNIV_MEM_ASSERT_RW(page_zip->data, page_zip_get_size(page_zip));
  UNIV_MEM_ASSERT_RW(rec, rec_offs_data_size(offsets));
  UNIV_MEM_ASSERT_RW(rec - rec_offs_extra_size(offsets),
                     rec_offs_extra_size(offsets));

  blob_no = page_zip_get_n_prev_extern(page_zip, rec, index) +
            rec_get_n_extern_new(rec, index, n);
  ut_a(blob_no < page_zip->n_blobs);

  externs = page_zip->data + page_zip_get_size(page_zip) -
            (page_dir_get_n_heap(page) - PAGE_HEAP_NO_USER_LOW) *
                PAGE_ZIP_CLUST_LEAF_SLOT_SIZE;

  field = rec_get_nth_field(rec, offsets, n, &len);

  externs -= (blob_no + 1) * BTR_EXTERN_FIELD_REF_SIZE;
  field += len - BTR_EXTERN_FIELD_REF_SIZE;

  memcpy(externs, field, BTR_EXTERN_FIELD_REF_SIZE);

#ifdef UNIV_ZIP_DEBUG
  ut_a(page_zip_validate(page_zip, page, index));
#endif /* UNIV_ZIP_DEBUG */

  if (mtr) {
#ifndef UNIV_HOTBACKUP
    byte *log_ptr = mlog_open(mtr, 11 + 2 + 2 + BTR_EXTERN_FIELD_REF_SIZE);
    if (UNIV_UNLIKELY(!log_ptr)) {
      return;
    }

    log_ptr = mlog_write_initial_log_record_fast(
        (byte *)field, MLOG_ZIP_WRITE_BLOB_PTR, log_ptr, mtr);
    mach_write_to_2(log_ptr, page_offset(field));
    log_ptr += 2;
    mach_write_to_2(log_ptr, externs - page_zip->data);
    log_ptr += 2;
    memcpy(log_ptr, externs, BTR_EXTERN_FIELD_REF_SIZE);
    log_ptr += BTR_EXTERN_FIELD_REF_SIZE;
    mlog_close(mtr, log_ptr);
#endif /* !UNIV_HOTBACKUP */
  }
}

/** Parses a log record of writing the node pointer of a record.
 @return end of log record or NULL */
byte *page_zip_parse_write_node_ptr(
    byte *ptr,                /*!< in: redo log buffer */
    byte *end_ptr,            /*!< in: redo log buffer end */
    page_t *page,             /*!< in/out: uncompressed page */
    page_zip_des_t *page_zip) /*!< in/out: compressed page */
{
  ulint offset;
  ulint z_offset;

  ut_ad(ptr != NULL);
  ut_ad(end_ptr != NULL);
  ut_ad(!page == !page_zip);

  if (UNIV_UNLIKELY(end_ptr < ptr + (2 + 2 + REC_NODE_PTR_SIZE))) {
    return (NULL);
  }

  offset = mach_read_from_2(ptr);
  z_offset = mach_read_from_2(ptr + 2);

  if (offset < PAGE_ZIP_START || offset >= UNIV_PAGE_SIZE ||
      z_offset >= UNIV_PAGE_SIZE) {
  corrupt:
    recv_sys->found_corrupt_log = TRUE;

    return (NULL);
  }

  if (page) {
    byte *storage_end;
    byte *field;
    byte *storage;
    ulint heap_no;

    if (!page_zip || page_is_leaf(page)) {
      goto corrupt;
    }

#ifdef UNIV_ZIP_DEBUG
    ut_a(page_zip_validate(page_zip, page, NULL));
#endif /* UNIV_ZIP_DEBUG */

    field = page + offset;
    storage = page_zip->data + z_offset;

    storage_end = page_zip_dir_start(page_zip);

    heap_no = 1 + (storage_end - storage) / REC_NODE_PTR_SIZE;

    if (UNIV_UNLIKELY((storage_end - storage) % REC_NODE_PTR_SIZE) ||
        UNIV_UNLIKELY(heap_no < PAGE_HEAP_NO_USER_LOW) ||
        UNIV_UNLIKELY(heap_no >= page_dir_get_n_heap(page))) {
      goto corrupt;
    }

    memcpy(field, ptr + 4, REC_NODE_PTR_SIZE);
    memcpy(storage, ptr + 4, REC_NODE_PTR_SIZE);

#ifdef UNIV_ZIP_DEBUG
    ut_a(page_zip_validate(page_zip, page, NULL));
#endif /* UNIV_ZIP_DEBUG */
  }

  return (ptr + (2 + 2 + REC_NODE_PTR_SIZE));
}

/** Write the node pointer of a record on a non-leaf compressed page. */
void page_zip_write_node_ptr(
    page_zip_des_t *page_zip, /*!< in/out: compressed page */
    byte *rec,                /*!< in/out: record */
    ulint size,               /*!< in: data size of rec */
    ulint ptr,                /*!< in: node pointer */
    mtr_t *mtr)               /*!< in: mini-transaction, or NULL */
{
  byte *field;
  byte *storage;
#ifdef UNIV_DEBUG
  page_t *page = page_align(rec);
#endif /* UNIV_DEBUG */

  ut_ad(page_simple_validate_new(page));
  ut_ad(page_zip_simple_validate(page_zip));
  ut_ad(page_zip_get_size(page_zip) > PAGE_DATA + page_zip_dir_size(page_zip));
  ut_ad(page_rec_is_comp(rec));

  ut_ad(page_zip->m_start >= PAGE_DATA);
  ut_ad(page_zip_header_cmp(page_zip, page));

  ut_ad(!page_is_leaf(page));

  UNIV_MEM_ASSERT_RW(page_zip->data, page_zip_get_size(page_zip));
  UNIV_MEM_ASSERT_RW(rec, size);

  storage = page_zip_dir_start(page_zip) -
            (rec_get_heap_no_new(rec) - 1) * REC_NODE_PTR_SIZE;
  field = rec + size - REC_NODE_PTR_SIZE;

#if defined UNIV_DEBUG || defined UNIV_ZIP_DEBUG
  ut_a(!memcmp(storage, field, REC_NODE_PTR_SIZE));
#endif /* UNIV_DEBUG || UNIV_ZIP_DEBUG */
#if REC_NODE_PTR_SIZE != 4
#error "REC_NODE_PTR_SIZE != 4"
#endif
  mach_write_to_4(field, ptr);
  memcpy(storage, field, REC_NODE_PTR_SIZE);

  if (mtr) {
#ifndef UNIV_HOTBACKUP
    byte *log_ptr = mlog_open(mtr, 11 + 2 + 2 + REC_NODE_PTR_SIZE);
    if (UNIV_UNLIKELY(!log_ptr)) {
      return;
    }

    log_ptr = mlog_write_initial_log_record_fast(field, MLOG_ZIP_WRITE_NODE_PTR,
                                                 log_ptr, mtr);
    mach_write_to_2(log_ptr, page_offset(field));
    log_ptr += 2;
    mach_write_to_2(log_ptr, storage - page_zip->data);
    log_ptr += 2;
    memcpy(log_ptr, field, REC_NODE_PTR_SIZE);
    log_ptr += REC_NODE_PTR_SIZE;
    mlog_close(mtr, log_ptr);
#endif /* !UNIV_HOTBACKUP */
  }
}

/** Write the trx_id and roll_ptr of a record on a B-tree leaf node page. */
void page_zip_write_trx_id_and_roll_ptr(
    page_zip_des_t *page_zip, /*!< in/out: compressed page */
    byte *rec,                /*!< in/out: record */
    const ulint *offsets,     /*!< in: rec_get_offsets(rec, index) */
    ulint trx_id_col,         /*!< in: column number of TRX_ID in rec */
    trx_id_t trx_id,          /*!< in: transaction identifier */
    roll_ptr_t roll_ptr)      /*!< in: roll_ptr */
{
  byte *field;
  byte *storage;
#ifdef UNIV_DEBUG
  page_t *page = page_align(rec);
#endif /* UNIV_DEBUG */
  ulint len;

  ut_ad(page_simple_validate_new(page));
  ut_ad(page_zip_simple_validate(page_zip));
  ut_ad(page_zip_get_size(page_zip) > PAGE_DATA + page_zip_dir_size(page_zip));
  ut_ad(rec_offs_validate(rec, NULL, offsets));
  ut_ad(rec_offs_comp(offsets));

  ut_ad(page_zip->m_start >= PAGE_DATA);
  ut_ad(page_zip_header_cmp(page_zip, page));

  ut_ad(page_is_leaf(page));

  UNIV_MEM_ASSERT_RW(page_zip->data, page_zip_get_size(page_zip));

  storage =
      page_zip_dir_start(page_zip) -
      (rec_get_heap_no_new(rec) - 1) * (DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN);

  field = const_cast<byte *>(rec_get_nth_field(rec, offsets, trx_id_col, &len));
  ut_ad(len == DATA_TRX_ID_LEN);
  ut_ad(field + DATA_TRX_ID_LEN ==
        rec_get_nth_field(rec, offsets, trx_id_col + 1, &len));
  ut_ad(len == DATA_ROLL_PTR_LEN);
#if defined UNIV_DEBUG || defined UNIV_ZIP_DEBUG
  ut_a(!memcmp(storage, field, DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN));
#endif /* UNIV_DEBUG || UNIV_ZIP_DEBUG */
  mach_write_to_6(field, trx_id);
  mach_write_to_7(field + DATA_TRX_ID_LEN, roll_ptr);
  memcpy(storage, field, DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN);

  UNIV_MEM_ASSERT_RW(rec, rec_offs_data_size(offsets));
  UNIV_MEM_ASSERT_RW(rec - rec_offs_extra_size(offsets),
                     rec_offs_extra_size(offsets));
  UNIV_MEM_ASSERT_RW(page_zip->data, page_zip_get_size(page_zip));
}

/** Clear an area on the uncompressed and compressed page.
 Do not clear the data payload, as that would grow the modification log. */
static void page_zip_clear_rec(
    page_zip_des_t *page_zip,  /*!< in/out: compressed page */
    byte *rec,                 /*!< in: record to clear */
    const dict_index_t *index, /*!< in: index of rec */
    const ulint *offsets)      /*!< in: rec_get_offsets(rec, index) */
{
  ulint heap_no;
  page_t *page = page_align(rec);
  byte *storage;
  byte *field;
  ulint len;
  /* page_zip_validate() would fail here if a record
  containing externally stored columns is being deleted. */
  ut_ad(rec_offs_validate(rec, index, offsets));
  ut_ad(!page_zip_dir_find(page_zip, page_offset(rec)));
  ut_ad(page_zip_dir_find_free(page_zip, page_offset(rec)));
  ut_ad(page_zip_header_cmp(page_zip, page));

  heap_no = rec_get_heap_no_new(rec);
  ut_ad(heap_no >= PAGE_HEAP_NO_USER_LOW);

  UNIV_MEM_ASSERT_RW(page_zip->data, page_zip_get_size(page_zip));
  UNIV_MEM_ASSERT_RW(rec, rec_offs_data_size(offsets));
  UNIV_MEM_ASSERT_RW(rec - rec_offs_extra_size(offsets),
                     rec_offs_extra_size(offsets));

  if (!page_is_leaf(page)) {
    /* Clear node_ptr. On the compressed page,
    there is an array of node_ptr immediately before the
    dense page directory, at the very end of the page. */
    storage = page_zip_dir_start(page_zip);
    ut_ad(dict_index_get_n_unique_in_tree_nonleaf(index) ==
          rec_offs_n_fields(offsets) - 1);
    field =
        rec_get_nth_field(rec, offsets, rec_offs_n_fields(offsets) - 1, &len);
    ut_ad(len == REC_NODE_PTR_SIZE);

    ut_ad(!rec_offs_any_extern(offsets));
    memset(field, 0, REC_NODE_PTR_SIZE);
    memset(storage - (heap_no - 1) * REC_NODE_PTR_SIZE, 0, REC_NODE_PTR_SIZE);
  } else if (index->is_clustered()) {
    /* Clear trx_id and roll_ptr. On the compressed page,
    there is an array of these fields immediately before the
    dense page directory, at the very end of the page. */
    const ulint trx_id_pos =
        dict_col_get_clust_pos(index->table->get_sys_col(DATA_TRX_ID), index);
    storage = page_zip_dir_start(page_zip);
    field = rec_get_nth_field(rec, offsets, trx_id_pos, &len);
    ut_ad(len == DATA_TRX_ID_LEN);

    memset(field, 0, DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN);
    memset(storage - (heap_no - 1) * (DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN), 0,
           DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN);

    if (rec_offs_any_extern(offsets)) {
      ulint i;

      for (i = rec_offs_n_fields(offsets); i--;) {
        /* Clear all BLOB pointers in order to make
        page_zip_validate() pass. */
        if (rec_offs_nth_extern(offsets, i)) {
          field = rec_get_nth_field(rec, offsets, i, &len);
          ut_ad(len == BTR_EXTERN_FIELD_REF_SIZE);
          memset(field + len - BTR_EXTERN_FIELD_REF_SIZE, 0,
                 BTR_EXTERN_FIELD_REF_SIZE);
        }
      }
    }
  } else {
    ut_ad(!rec_offs_any_extern(offsets));
  }

#ifdef UNIV_ZIP_DEBUG
  ut_a(page_zip_validate(page_zip, page, index));
#endif /* UNIV_ZIP_DEBUG */
}





/** Insert a record to the dense page directory. */
void page_zip_dir_insert(
    page_zip_des_t *page_zip, /*!< in/out: compressed page */
    const byte *prev_rec,     /*!< in: record after which to insert */
    const byte *free_rec,     /*!< in: record from which rec was
                             allocated, or NULL */
    byte *rec)                /*!< in: record to insert */
{
  ulint n_dense;
  byte *slot_rec;
  byte *slot_free;

  ut_ad(prev_rec != rec);
  ut_ad(page_rec_get_next((rec_t *)prev_rec) == rec);
  ut_ad(page_zip_simple_validate(page_zip));

  UNIV_MEM_ASSERT_RW(page_zip->data, page_zip_get_size(page_zip));

  if (page_rec_is_infimum(prev_rec)) {
    /* Use the first slot. */
    slot_rec = page_zip->data + page_zip_get_size(page_zip);
  } else {
    byte *end = page_zip->data + page_zip_get_size(page_zip);
    byte *start = end - page_zip_dir_user_size(page_zip);

    if (UNIV_LIKELY(!free_rec)) {
      /* PAGE_N_RECS was already incremented
      in page_cur_insert_rec_zip(), but the
      dense directory slot at that position
      contains garbage.  Skip it. */
      start += PAGE_ZIP_DIR_SLOT_SIZE;
    }

    slot_rec = page_zip_dir_find_low(start, end, page_offset(prev_rec));
    ut_a(slot_rec);
  }

  /* Read the old n_dense (n_heap may have been incremented). */
  n_dense = page_dir_get_n_heap(page_zip->data) - (PAGE_HEAP_NO_USER_LOW + 1);

  if (UNIV_LIKELY_NULL(free_rec)) {
    /* The record was allocated from the free list.
    Shift the dense directory only up to that slot.
    Note that in this case, n_dense is actually
    off by one, because page_cur_insert_rec_zip()
    did not increment n_heap. */
    ut_ad(rec_get_heap_no_new(rec) < n_dense + 1 + PAGE_HEAP_NO_USER_LOW);
    ut_ad(rec >= free_rec);
    slot_free = page_zip_dir_find(page_zip, page_offset(free_rec));
    ut_ad(slot_free);
    slot_free += PAGE_ZIP_DIR_SLOT_SIZE;
  } else {
    /* The record was allocated from the heap.
    Shift the entire dense directory. */
    ut_ad(rec_get_heap_no_new(rec) == n_dense + PAGE_HEAP_NO_USER_LOW);

    /* Shift to the end of the dense page directory. */
    slot_free = page_zip->data + page_zip_get_size(page_zip) -
                PAGE_ZIP_DIR_SLOT_SIZE * n_dense;
  }

  /* Shift the dense directory to allocate place for rec. */
  memmove(slot_free - PAGE_ZIP_DIR_SLOT_SIZE, slot_free, slot_rec - slot_free);

  /* Write the entry for the inserted record.
  The "owned" and "deleted" flags must be zero. */
  mach_write_to_2(slot_rec - PAGE_ZIP_DIR_SLOT_SIZE, page_offset(rec));
}

/** Shift the dense page directory when a record is deleted.
@param[in,out]	page_zip	compressed page
@param[in]	rec		deleted record
@param[in]	index		index of rec
@param[in]	offsets		rec_get_offsets(rec)
@param[in]	free		previous start of the free list */
void page_zip_dir_delete(page_zip_des_t *page_zip, byte *rec,
                         const dict_index_t *index, const ulint *offsets,
                         const byte *free) {
  byte *slot_rec;
  byte *slot_free;
  ulint n_ext;
  page_t *page = page_align(rec);

  ut_ad(rec_offs_validate(rec, index, offsets));
  ut_ad(rec_offs_comp(offsets));

  UNIV_MEM_ASSERT_RW(page_zip->data, page_zip_get_size(page_zip));
  UNIV_MEM_ASSERT_RW(rec, rec_offs_data_size(offsets));
  UNIV_MEM_ASSERT_RW(rec - rec_offs_extra_size(offsets),
                     rec_offs_extra_size(offsets));

  slot_rec = page_zip_dir_find(page_zip, page_offset(rec));

  ut_a(slot_rec);

  /* This could not be done before page_zip_dir_find(). */
  page_header_set_field(page, page_zip, PAGE_N_RECS,
                        (ulint)(page_get_n_recs(page) - 1));

  if (UNIV_UNLIKELY(!free)) {
    /* Make the last slot the start of the free list. */
    slot_free = page_zip->data + page_zip_get_size(page_zip) -
                PAGE_ZIP_DIR_SLOT_SIZE * (page_dir_get_n_heap(page_zip->data) -
                                          PAGE_HEAP_NO_USER_LOW);
  } else {
    slot_free = page_zip_dir_find_free(page_zip, page_offset(free));
    ut_a(slot_free < slot_rec);
    /* Grow the free list by one slot by moving the start. */
    slot_free += PAGE_ZIP_DIR_SLOT_SIZE;
  }

  if (UNIV_LIKELY(slot_rec > slot_free)) {
    memmove(slot_free + PAGE_ZIP_DIR_SLOT_SIZE, slot_free,
            slot_rec - slot_free);
  }

  /* Write the entry for the deleted record.
  The "owned" and "deleted" flags will be cleared. */
  mach_write_to_2(slot_free, page_offset(rec));

  if (!page_is_leaf(page) || !index->is_clustered()) {
    ut_ad(!rec_offs_any_extern(offsets));
    goto skip_blobs;
  }

  n_ext = rec_offs_n_extern(offsets);
  if (UNIV_UNLIKELY(n_ext)) {
    /* Shift and zero fill the array of BLOB pointers. */
    ulint blob_no;
    byte *externs;
    byte *ext_end;

    blob_no = page_zip_get_n_prev_extern(page_zip, rec, index);
    ut_a(blob_no + n_ext <= page_zip->n_blobs);

    externs = page_zip->data + page_zip_get_size(page_zip) -
              (page_dir_get_n_heap(page) - PAGE_HEAP_NO_USER_LOW) *
                  PAGE_ZIP_CLUST_LEAF_SLOT_SIZE;

    ext_end = externs - page_zip->n_blobs * BTR_EXTERN_FIELD_REF_SIZE;
    externs -= blob_no * BTR_EXTERN_FIELD_REF_SIZE;

    page_zip->n_blobs -= static_cast<unsigned>(n_ext);
    /* Shift and zero fill the array. */
    memmove(ext_end + n_ext * BTR_EXTERN_FIELD_REF_SIZE, ext_end,
            (page_zip->n_blobs - blob_no) * BTR_EXTERN_FIELD_REF_SIZE);
    memset(ext_end, 0, n_ext * BTR_EXTERN_FIELD_REF_SIZE);
  }

skip_blobs:
  /* The compression algorithm expects info_bits and n_owned
  to be 0 for deleted records. */
  rec[-REC_N_NEW_EXTRA_BYTES] = 0; /* info_bits and n_owned */

  page_zip_clear_rec(page_zip, rec, index, offsets);
}


/** Parses a log record of writing to the header of a page.
 @return end of log record or NULL */
byte *page_zip_parse_write_header(
    byte *ptr,                /*!< in: redo log buffer */
    byte *end_ptr,            /*!< in: redo log buffer end */
    page_t *page,             /*!< in/out: uncompressed page */
    page_zip_des_t *page_zip) /*!< in/out: compressed page */
{
  ulint offset;
  ulint len;

  ut_ad(ptr != NULL);
  ut_ad(end_ptr != NULL);
  ut_ad(!page == !page_zip);

  if (UNIV_UNLIKELY(end_ptr < ptr + (1 + 1))) {
    return (NULL);
  }

  offset = (ulint)*ptr++;
  len = (ulint)*ptr++;

  if (len == 0 || offset + len >= PAGE_DATA) {
  corrupt:
    recv_sys->found_corrupt_log = TRUE;

    return (NULL);
  }

  if (end_ptr < ptr + len) {
    return (NULL);
  }

  if (page) {
    if (!page_zip) {
      goto corrupt;
    }
#ifdef UNIV_ZIP_DEBUG
    ut_a(page_zip_validate(page_zip, page, NULL));
#endif /* UNIV_ZIP_DEBUG */

    memcpy(page + offset, ptr, len);
    memcpy(page_zip->data + offset, ptr, len);

#ifdef UNIV_ZIP_DEBUG
    ut_a(page_zip_validate(page_zip, page, NULL));
#endif /* UNIV_ZIP_DEBUG */
  }

  return (ptr + len);
}



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
    mtr_t *mtr)          /*!< in: mini-transaction */
{
#ifndef UNIV_HOTBACKUP
  buf_pool_t *buf_pool = buf_pool_from_block(block);
#endif /* !UNIV_HOTBACKUP */
  page_zip_des_t *page_zip = buf_block_get_page_zip(block);
  page_t *page = buf_block_get_frame(block);
  buf_block_t *temp_block;
  page_t *temp_page;

#ifndef UNIV_HOTBACKUP
  ut_ad(mtr_memo_contains(mtr, block, MTR_MEMO_PAGE_X_FIX));
#endif /* !UNIV_HOTBACKUP */
  ut_ad(page_is_comp(page));
  ut_ad(!dict_index_is_ibuf(index));
  ut_ad(!index->table->is_temporary());
  /* Note that page_zip_validate(page_zip, page, index) may fail here. */
  UNIV_MEM_ASSERT_RW(page, UNIV_PAGE_SIZE);
  UNIV_MEM_ASSERT_RW(page_zip->data, page_zip_get_size(page_zip));

  /* Disable logging */
  mtr_log_t log_mode = mtr_set_log_mode(mtr, MTR_LOG_NONE);

#ifndef UNIV_HOTBACKUP
  temp_block = buf_block_alloc(buf_pool);
  btr_search_drop_page_hash_index(block);
#else  /* !UNIV_HOTBACKUP */
  temp_block = back_block2;
#endif /* !UNIV_HOTBACKUP */
  temp_page = temp_block->frame;

  /* Copy the old page to temporary space */
  buf_frame_copy(temp_page, page);

  /* Recreate the page: note that global data on page (possible
  segment headers, next page-field, etc.) is preserved intact */

  page_create(block, mtr, TRUE, fil_page_get_type(page));

  /* Copy the records from the temporary space to the recreated page;
  do not copy the lock bits yet */

  page_copy_rec_list_end_no_locks(block, temp_block,
                                  page_get_infimum_rec(temp_page), index, mtr);

  /* Temp-Tables are not shared across connection and so we avoid
  locking of temp-tables as there would be no 2 trx trying to
  operate on same temp-table in parallel.
  max_trx_id is use to track which all trxs wrote to the page
  in parallel but in case of temp-table this can is not needed. */
  if (!index->is_clustered() && page_is_leaf(temp_page)) {
    /* Copy max trx id to recreated page */
    trx_id_t max_trx_id = page_get_max_trx_id(temp_page);
    page_set_max_trx_id(block, NULL, max_trx_id, NULL);
    ut_ad(max_trx_id != 0);
  }

  /* Restore logging. */
  mtr_set_log_mode(mtr, log_mode);

  if (!page_zip_compress(page_zip, page, index, page_zip_level, mtr)) {
#ifndef UNIV_HOTBACKUP
    buf_block_free(temp_block);
#endif /* !UNIV_HOTBACKUP */
    return (FALSE);
  }

  lock_move_reorganize_page(block, temp_block);

#ifndef UNIV_HOTBACKUP
  buf_block_free(temp_block);
#endif /* !UNIV_HOTBACKUP */
  return (TRUE);
}

#ifndef UNIV_HOTBACKUP
/** Copy the records of a page byte for byte.  Do not copy the page header
 or trailer, except those B-tree header fields that are directly
 related to the storage of records.  Also copy PAGE_MAX_TRX_ID.
 NOTE: The caller must update the lock table and the adaptive hash index. */
void page_zip_copy_recs(
    page_zip_des_t *page_zip,      /*!< out: copy of src_zip
                                   (n_blobs, m_start, m_end,
                                   m_nonempty, data[0..size-1]) */
    page_t *page,                  /*!< out: copy of src */
    const page_zip_des_t *src_zip, /*!< in: compressed page */
    const page_t *src,             /*!< in: page */
    dict_index_t *index,           /*!< in: index of the B-tree */
    mtr_t *mtr)                    /*!< in: mini-transaction */
{
  ut_ad(!index->table->is_temporary());
  ut_ad(mtr_memo_contains_page(mtr, page, MTR_MEMO_PAGE_X_FIX));
  ut_ad(mtr_memo_contains_page(mtr, src, MTR_MEMO_PAGE_X_FIX));
  ut_ad(!dict_index_is_ibuf(index));
#ifdef UNIV_ZIP_DEBUG
  /* The B-tree operations that call this function may set
  FIL_PAGE_PREV or PAGE_LEVEL, causing a temporary min_rec_flag
  mismatch.  A strict page_zip_validate() will be executed later
  during the B-tree operations. */
  ut_a(page_zip_validate_low(src_zip, src, index, TRUE));
#endif /* UNIV_ZIP_DEBUG */
  ut_a(page_zip_get_size(page_zip) == page_zip_get_size(src_zip));
  if (UNIV_UNLIKELY(src_zip->n_blobs)) {
    ut_a(page_is_leaf(src));
    ut_a(index->is_clustered());
  }

  /* The PAGE_MAX_TRX_ID must be set on leaf pages of secondary
  indexes.  It does not matter on other pages. */
  ut_a(index->is_clustered() || !page_is_leaf(src) || page_get_max_trx_id(src));

  UNIV_MEM_ASSERT_W(page, UNIV_PAGE_SIZE);
  UNIV_MEM_ASSERT_W(page_zip->data, page_zip_get_size(page_zip));
  UNIV_MEM_ASSERT_RW(src, UNIV_PAGE_SIZE);
  UNIV_MEM_ASSERT_RW(src_zip->data, page_zip_get_size(page_zip));

  /* Copy those B-tree page header fields that are related to
  the records stored in the page.  Also copy the field
  PAGE_MAX_TRX_ID.  Skip the rest of the page header and
  trailer.  On the compressed page, there is no trailer. */
#if PAGE_MAX_TRX_ID + 8 != PAGE_HEADER_PRIV_END
#error "PAGE_MAX_TRX_ID + 8 != PAGE_HEADER_PRIV_END"
#endif
  memcpy(PAGE_HEADER + page, PAGE_HEADER + src, PAGE_HEADER_PRIV_END);
  memcpy(PAGE_DATA + page, PAGE_DATA + src,
         UNIV_PAGE_SIZE - PAGE_DATA - FIL_PAGE_DATA_END);
  memcpy(PAGE_HEADER + page_zip->data, PAGE_HEADER + src_zip->data,
         PAGE_HEADER_PRIV_END);
  memcpy(PAGE_DATA + page_zip->data, PAGE_DATA + src_zip->data,
         page_zip_get_size(page_zip) - PAGE_DATA);

  /* Copy all fields of src_zip to page_zip, except the pointer
  to the compressed data page. */
  {
    page_zip_t *data = page_zip->data;
    memcpy(page_zip, src_zip, sizeof *page_zip);
    page_zip->data = data;
  }
  ut_ad(page_zip_get_trailer_len(page_zip, index->is_clustered()) +
            page_zip->m_end <
        page_zip_get_size(page_zip));

  if (!page_is_leaf(src) &&
      UNIV_UNLIKELY(mach_read_from_4(src + FIL_PAGE_PREV) == FIL_NULL) &&
      UNIV_LIKELY(mach_read_from_4(page + FIL_PAGE_PREV) != FIL_NULL)) {
    /* Clear the REC_INFO_MIN_REC_FLAG of the first user record. */
    ulint offs = rec_get_next_offs(page + PAGE_NEW_INFIMUM, TRUE);
    if (UNIV_LIKELY(offs != PAGE_NEW_SUPREMUM)) {
      rec_t *rec = page + offs;
      ut_a(rec[-REC_N_NEW_EXTRA_BYTES] & REC_INFO_MIN_REC_FLAG);
      rec[-REC_N_NEW_EXTRA_BYTES] &= ~REC_INFO_MIN_REC_FLAG;
    }
  }

#ifdef UNIV_ZIP_DEBUG
  ut_a(page_zip_validate(page_zip, page, index));
#endif /* UNIV_ZIP_DEBUG */
  page_zip_compress_write_log(page_zip, page, index, mtr);
}
#endif /* !UNIV_HOTBACKUP */

/** Parses a log record of compressing an index page.
 @return end of log record or NULL */
byte *page_zip_parse_compress(
    byte *ptr,                /*!< in: buffer */
    byte *end_ptr,            /*!< in: buffer end */
    page_t *page,             /*!< out: uncompressed page */
    page_zip_des_t *page_zip) /*!< out: compressed page */
{
  ulint size;
  ulint trailer_size;

  ut_ad(ptr != NULL);
  ut_ad(end_ptr != NULL);
  ut_ad(!page == !page_zip);

  if (UNIV_UNLIKELY(ptr + (2 + 2) > end_ptr)) {
    return (NULL);
  }

  size = mach_read_from_2(ptr);
  ptr += 2;
  trailer_size = mach_read_from_2(ptr);
  ptr += 2;

  if (UNIV_UNLIKELY(ptr + 8 + size + trailer_size > end_ptr)) {
    return (NULL);
  }

  if (page) {
    if (!page_zip || page_zip_get_size(page_zip) < size) {
    corrupt:
      recv_sys->found_corrupt_log = TRUE;

      return (NULL);
    }

    memcpy(page_zip->data + FIL_PAGE_PREV, ptr, 4);
    memcpy(page_zip->data + FIL_PAGE_NEXT, ptr + 4, 4);
    memcpy(page_zip->data + FIL_PAGE_TYPE, ptr + 8, size);
    memset(page_zip->data + FIL_PAGE_TYPE + size, 0,
           page_zip_get_size(page_zip) - trailer_size - (FIL_PAGE_TYPE + size));
    memcpy(page_zip->data + page_zip_get_size(page_zip) - trailer_size,
           ptr + 8 + size, trailer_size);

    if (UNIV_UNLIKELY(!page_zip_decompress(page_zip, page, TRUE))) {
      goto corrupt;
    }
  }

  return (ptr + 8 + size + trailer_size);
}
