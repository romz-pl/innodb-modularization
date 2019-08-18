#include <innodb/page/page_zip_compress.h>

#include <innodb/dict_mem/dict_index_get_n_fields.h>
#include <innodb/dict_mem/dict_index_get_n_unique_in_tree_nonleaf.h>
#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_types/index_id_t.h>
#include <innodb/disk/fil_page_index_page_check.h>
#include <innodb/memory/mem_heap_zalloc.h>
#include <innodb/memory_check/memory_check.h>
#include <innodb/monitor/MONITOR_INC.h>
#include <innodb/monitor/monitor_id_t.h>
#include <innodb/mtr/mtr_t.h>
#include <innodb/page/LOGFILE.h>
#include <innodb/page/LOGFILE.h>
#include <innodb/page/flag.h>
#include <innodb/page/header.h>
#include <innodb/page/infimum_data.h>
#include <innodb/page/infimum_extra.h>
#include <innodb/page/page_dir_get_n_heap.h>
#include <innodb/page/page_get_n_recs.h>
#include <innodb/page/page_get_n_recs.h>
#include <innodb/page/page_header_get_field.h>
#include <innodb/page/page_is_comp.h>
#include <innodb/page/page_is_empty.h>
#include <innodb/page/page_is_leaf.h>
#include <innodb/page/page_zip_compress_clust.h>
#include <innodb/page/page_zip_compress_node_ptrs.h>
#include <innodb/page/page_zip_compress_sec.h>
#include <innodb/page/page_zip_compress_write_log.h>
#include <innodb/page/page_zip_des_t.h>
#include <innodb/page/page_zip_dir_encode.h>
#include <innodb/page/page_zip_fields_encode.h>
#include <innodb/page/page_zip_get_size.h>
#include <innodb/page/page_zip_set_alloc.h>
#include <innodb/page/page_zip_stat.h>
#include <innodb/page/page_zip_stat_per_index.h>
#include <innodb/page/page_zip_stat_per_index_mutex.h>
#include <innodb/page/supremum_extra_data.h>
#include <innodb/record/rec_get_next_offs.h>
#include <innodb/sync_mutex/mutex_enter.h>
#include <innodb/sync_mutex/mutex_exit.h>
#include <innodb/time/ut_time_us.h>
#include <innodb/univ/rec_t.h>


#include <zlib.h>

static_assert(DATA_TRX_ID_LEN == 6, "DATA_TRX_ID_LEN != 6");
static_assert(DATA_ROLL_PTR_LEN == 7, "DATA_ROLL_PTR_LEN != 7");
static_assert(DATA_TRX_ID + 1 == DATA_ROLL_PTR, "DATA_TRX_ID invalid value!");

extern bool srv_cmp_per_index_enabled;

void dict_index_zip_failure(dict_index_t *index);
void dict_index_zip_success(dict_index_t *index);


/** Compress a page.
 @return true on success, false on failure; page_zip will be left
 intact on failure. */
ibool page_zip_compress(page_zip_des_t *page_zip, /*!< in: size; out: data,
                                                  n_blobs, m_start, m_end,
                                                  m_nonempty */
                        const page_t *page,       /*!< in: uncompressed page */
                        dict_index_t *index,      /*!< in: index tree */
                        ulint level,              /*!< in: commpression level */
                        mtr_t *mtr)               /*!< in/out: mini-transaction,
                                                  or NULL */
{
  z_stream c_stream;
  int err;
  ulint n_fields; /* number of index fields
                  needed */
  byte *fields;   /*!< index field information */
  byte *buf;      /*!< compressed payload of the
                  page */
  byte *buf_end;  /* end of buf */
  ulint n_dense;
  ulint slot_size;    /* amount of uncompressed bytes
                      per record */
  const rec_t **recs; /*!< dense page directory,
                      sorted by address */
  mem_heap_t *heap;
  ulint trx_id_col = ULINT_UNDEFINED;
  ulint n_blobs = 0;
  byte *storage; /* storage of uncompressed
                 columns */
#ifndef UNIV_HOTBACKUP
  uintmax_t usec = ut_time_us(NULL);
#endif /* !UNIV_HOTBACKUP */
#ifdef PAGE_ZIP_COMPRESS_DBG
  FILE *logfile = NULL;
#endif
#ifndef UNIV_HOTBACKUP
  /* A local copy of srv_cmp_per_index_enabled to avoid reading that
  variable multiple times in this function since it can be changed at
  anytime. */
  bool cmp_per_index_enabled;
  cmp_per_index_enabled = srv_cmp_per_index_enabled;
#endif /* !UNIV_HOTBACKUP */

  ut_a(page_is_comp(page));
  ut_a(fil_page_index_page_check(page));
  ut_ad(page_simple_validate_new((page_t *)page));
  ut_ad(page_zip_simple_validate(page_zip));
  ut_ad(dict_table_is_comp(index->table));
  ut_ad(!dict_index_is_ibuf(index));

  UNIV_MEM_ASSERT_RW(page, UNIV_PAGE_SIZE);

  /* Check the data that will be omitted. */
  ut_a(!memcmp(page + (PAGE_NEW_INFIMUM - REC_N_NEW_EXTRA_BYTES), infimum_extra,
               sizeof infimum_extra));
  ut_a(!memcmp(page + PAGE_NEW_INFIMUM, infimum_data, sizeof infimum_data));
  ut_a(page[PAGE_NEW_SUPREMUM - REC_N_NEW_EXTRA_BYTES]
       /* info_bits == 0, n_owned <= max */
       <= PAGE_DIR_SLOT_MAX_N_OWNED);
  ut_a(!memcmp(page + (PAGE_NEW_SUPREMUM - REC_N_NEW_EXTRA_BYTES + 1),
               supremum_extra_data, sizeof supremum_extra_data));

  if (page_is_empty(page)) {
    ut_a(rec_get_next_offs(page + PAGE_NEW_INFIMUM, TRUE) == PAGE_NEW_SUPREMUM);
  }

  if (page_is_leaf(page)) {
    n_fields = dict_index_get_n_fields(index);
  } else {
    n_fields = dict_index_get_n_unique_in_tree_nonleaf(index);
  }

  index_id_t ind_id(index->space, index->id);

  /* The dense directory excludes the infimum and supremum records. */
  n_dense = page_dir_get_n_heap(page) - PAGE_HEAP_NO_USER_LOW;
#ifdef PAGE_ZIP_COMPRESS_DBG
  if (UNIV_UNLIKELY(page_zip_compress_dbg)) {
    ib::info(ER_IB_MSG_915)
        << "compress " << static_cast<void *>(page_zip) << " "
        << static_cast<const void *>(page) << " " << page_is_leaf(page) << " "
        << n_fields << " " << n_dense;
  }

  if (UNIV_UNLIKELY(page_zip_compress_log)) {
    /* Create a log file for every compression attempt. */
    char logfilename[9];
    snprintf(logfilename, sizeof logfilename, "%08x", page_zip_compress_log++);
    logfile = fopen(logfilename, "wb");

    if (logfile) {
      /* Write the uncompressed page to the log. */
      if (fwrite(page, 1, UNIV_PAGE_SIZE, logfile) != UNIV_PAGE_SIZE) {
        perror("fwrite");
      }
      /* Record the compressed size as zero.
      This will be overwritten at successful exit. */
      putc(0, logfile);
      putc(0, logfile);
      putc(0, logfile);
      putc(0, logfile);
    }
  }
#endif /* PAGE_ZIP_COMPRESS_DBG */
#ifndef UNIV_HOTBACKUP
  page_zip_stat[page_zip->ssize - 1].compressed++;

  if (cmp_per_index_enabled) {
    mutex_enter(&page_zip_stat_per_index_mutex);
    page_zip_stat_per_index[ind_id].compressed++;
    mutex_exit(&page_zip_stat_per_index_mutex);
  }
#endif /* !UNIV_HOTBACKUP */

  if (UNIV_UNLIKELY(n_dense * PAGE_ZIP_DIR_SLOT_SIZE >=
                    page_zip_get_size(page_zip))) {
    goto err_exit;
  }

  MONITOR_INC(MONITOR_PAGE_COMPRESS);

  heap = mem_heap_create(page_zip_get_size(page_zip) +
                         n_fields * (2 + sizeof(ulint)) + REC_OFFS_HEADER_SIZE +
                         n_dense * ((sizeof *recs) - PAGE_ZIP_DIR_SLOT_SIZE) +
                         UNIV_PAGE_SIZE * 4 + (512 << MAX_MEM_LEVEL));

  recs = static_cast<const rec_t **>(
      mem_heap_zalloc(heap, n_dense * sizeof *recs));

  fields = static_cast<byte *>(mem_heap_alloc(heap, (n_fields + 1) * 2));

  buf = static_cast<byte *>(
      mem_heap_alloc(heap, page_zip_get_size(page_zip) - PAGE_DATA));

  buf_end = buf + page_zip_get_size(page_zip) - PAGE_DATA;

  /* Compress the data payload. */
  page_zip_set_alloc(&c_stream, heap);

  err = deflateInit2(&c_stream, static_cast<int>(level), Z_DEFLATED,
                     UNIV_PAGE_SIZE_SHIFT, MAX_MEM_LEVEL, Z_DEFAULT_STRATEGY);
  ut_a(err == Z_OK);

  c_stream.next_out = buf;

  /* Subtract the space reserved for uncompressed data. */
  /* Page header and the end marker of the modification log */
  c_stream.avail_out = static_cast<uInt>(buf_end - buf - 1);

  /* Dense page directory and uncompressed columns, if any */
  if (page_is_leaf(page)) {
    if (index->is_clustered()) {
      trx_id_col = index->get_sys_col_pos(DATA_TRX_ID);
      ut_ad(trx_id_col > 0);
      ut_ad(trx_id_col != ULINT_UNDEFINED);

      slot_size = PAGE_ZIP_DIR_SLOT_SIZE + DATA_TRX_ID_LEN + DATA_ROLL_PTR_LEN;

    } else {
      /* Signal the absence of trx_id
      in page_zip_fields_encode() */
      ut_ad(index->get_sys_col_pos(DATA_TRX_ID) == ULINT_UNDEFINED);
      trx_id_col = 0;
      slot_size = PAGE_ZIP_DIR_SLOT_SIZE;
    }
  } else {
    slot_size = PAGE_ZIP_DIR_SLOT_SIZE + REC_NODE_PTR_SIZE;
    trx_id_col = ULINT_UNDEFINED;
  }

  if (UNIV_UNLIKELY(c_stream.avail_out <=
                    n_dense * slot_size +
                        6 /* sizeof(zlib header and footer) */)) {
    goto zlib_error;
  }

  c_stream.avail_out -= static_cast<uInt>(n_dense * slot_size);
  c_stream.avail_in = static_cast<uInt>(
      page_zip_fields_encode(n_fields, index, trx_id_col, fields));
  c_stream.next_in = fields;

  if (UNIV_LIKELY(!trx_id_col)) {
    trx_id_col = ULINT_UNDEFINED;
  }

  UNIV_MEM_ASSERT_RW(c_stream.next_in, c_stream.avail_in);
  err = deflate(&c_stream, Z_FULL_FLUSH);
  if (err != Z_OK) {
    goto zlib_error;
  }

  ut_ad(!c_stream.avail_in);

  page_zip_dir_encode(page, buf_end, recs);

  c_stream.next_in = (byte *)page + PAGE_ZIP_START;

  storage = buf_end - n_dense * PAGE_ZIP_DIR_SLOT_SIZE;

  /* Compress the records in heap_no order. */
  if (UNIV_UNLIKELY(!n_dense)) {
  } else if (!page_is_leaf(page)) {
    /* This is a node pointer page. */
    err = page_zip_compress_node_ptrs(LOGFILE & c_stream, recs, n_dense, index,
                                      storage, heap);
    if (UNIV_UNLIKELY(err != Z_OK)) {
      goto zlib_error;
    }
  } else if (UNIV_LIKELY(trx_id_col == ULINT_UNDEFINED)) {
    /* This is a leaf page in a secondary index. */
    err = page_zip_compress_sec(LOGFILE & c_stream, recs, n_dense);
    if (UNIV_UNLIKELY(err != Z_OK)) {
      goto zlib_error;
    }
  } else {
    /* This is a leaf page in a clustered index. */
    err = page_zip_compress_clust(
        LOGFILE & c_stream, recs, n_dense, index, &n_blobs, trx_id_col,
        buf_end - PAGE_ZIP_DIR_SLOT_SIZE * page_get_n_recs(page), storage,
        heap);
    if (UNIV_UNLIKELY(err != Z_OK)) {
      goto zlib_error;
    }
  }

  /* Finish the compression. */
  ut_ad(!c_stream.avail_in);
  /* Compress any trailing garbage, in case the last record was
  allocated from an originally longer space on the free list,
  or the data of the last record from page_zip_compress_sec(). */
  c_stream.avail_in = static_cast<uInt>(
      page_header_get_field(page, PAGE_HEAP_TOP) - (c_stream.next_in - page));
  ut_a(c_stream.avail_in <= UNIV_PAGE_SIZE - PAGE_ZIP_START - PAGE_DIR);

  UNIV_MEM_ASSERT_RW(c_stream.next_in, c_stream.avail_in);
  err = deflate(&c_stream, Z_FINISH);

  if (UNIV_UNLIKELY(err != Z_STREAM_END)) {
  zlib_error:
    deflateEnd(&c_stream);
    mem_heap_free(heap);
  err_exit:
#ifdef PAGE_ZIP_COMPRESS_DBG
    if (logfile) {
      fclose(logfile);
    }
#endif /* PAGE_ZIP_COMPRESS_DBG */
#ifndef UNIV_HOTBACKUP
    if (page_is_leaf(page) && index) {
      dict_index_zip_failure(index);
    }

    uintmax_t time_diff = ut_time_us(NULL) - usec;
    page_zip_stat[page_zip->ssize - 1].compressed_usec += time_diff;
    if (cmp_per_index_enabled) {
      mutex_enter(&page_zip_stat_per_index_mutex);
      page_zip_stat_per_index[ind_id].compressed_usec += time_diff;
      mutex_exit(&page_zip_stat_per_index_mutex);
    }
#endif /* !UNIV_HOTBACKUP */
    return (FALSE);
  }

  err = deflateEnd(&c_stream);
  ut_a(err == Z_OK);

  ut_ad(buf + c_stream.total_out == c_stream.next_out);
  ut_ad((ulint)(storage - c_stream.next_out) >= c_stream.avail_out);

  /* Valgrind believes that zlib does not initialize some bits
  in the last 7 or 8 bytes of the stream.  Make Valgrind happy. */
  UNIV_MEM_VALID(buf, c_stream.total_out);

  /* Zero out the area reserved for the modification log.
  Space for the end marker of the modification log is not
  included in avail_out. */
  memset(c_stream.next_out, 0, c_stream.avail_out + 1 /* end marker */);

#ifdef UNIV_DEBUG
  page_zip->m_start =
#endif /* UNIV_DEBUG */
      page_zip->m_end = PAGE_DATA + c_stream.total_out;
  page_zip->m_nonempty = FALSE;
  page_zip->n_blobs = n_blobs;
  /* Copy those header fields that will not be written
  in buf_flush_init_for_writing() */
  memcpy(page_zip->data + FIL_PAGE_PREV, page + FIL_PAGE_PREV,
         FIL_PAGE_LSN - FIL_PAGE_PREV);
  memcpy(page_zip->data + FIL_PAGE_TYPE, page + FIL_PAGE_TYPE, 2);
  memcpy(page_zip->data + FIL_PAGE_DATA, page + FIL_PAGE_DATA,
         PAGE_DATA - FIL_PAGE_DATA);
  /* Copy the rest of the compressed page */
  memcpy(page_zip->data + PAGE_DATA, buf,
         page_zip_get_size(page_zip) - PAGE_DATA);
  mem_heap_free(heap);
#ifdef UNIV_ZIP_DEBUG
  ut_a(page_zip_validate(page_zip, page, index));
#endif /* UNIV_ZIP_DEBUG */

  if (mtr) {
#ifndef UNIV_HOTBACKUP
    page_zip_compress_write_log(page_zip, page, index, mtr);
#endif /* !UNIV_HOTBACKUP */
  }

  UNIV_MEM_ASSERT_RW(page_zip->data, page_zip_get_size(page_zip));

#ifdef PAGE_ZIP_COMPRESS_DBG
  if (logfile) {
    /* Record the compressed size of the block. */
    byte sz[4];
    mach_write_to_4(sz, c_stream.total_out);
    fseek(logfile, UNIV_PAGE_SIZE, SEEK_SET);
    if (fwrite(sz, 1, sizeof sz, logfile) != sizeof sz) {
      perror("fwrite");
    }
    fclose(logfile);
  }
#endif /* PAGE_ZIP_COMPRESS_DBG */
#ifndef UNIV_HOTBACKUP
  uintmax_t time_diff = ut_time_us(NULL) - usec;
  page_zip_stat[page_zip->ssize - 1].compressed_ok++;
  page_zip_stat[page_zip->ssize - 1].compressed_usec += time_diff;
  if (cmp_per_index_enabled) {
    mutex_enter(&page_zip_stat_per_index_mutex);
    page_zip_stat_per_index[ind_id].compressed_ok++;
    page_zip_stat_per_index[ind_id].compressed_usec += time_diff;
    mutex_exit(&page_zip_stat_per_index_mutex);
  }

  if (page_is_leaf(page)) {
    dict_index_zip_success(index);
  }
#endif /* !UNIV_HOTBACKUP */

  return (TRUE);
}
