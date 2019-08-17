#include <innodb/mtr/mlog_open_and_write_index.h>

#include <innodb/dict_mem/dict_index_get_n_fields.h>
#include <innodb/dict_mem/dict_index_get_n_unique_in_tree.h>
#include <innodb/dict_mem/dict_index_get_n_unique_in_tree_nonleaf.h>
#include <innodb/dict_mem/dict_index_is_spatial.h>
#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/mtr/mlog_close.h>
#include <innodb/mtr/mlog_open.h>
#include <innodb/mtr/mlog_write_initial_log_record_fast.h>
#include <innodb/mtr/mtr_t.h>
#include <innodb/page/page_align.h>
#include <innodb/page/page_is_leaf.h>
#include <innodb/page/page_rec_is_comp.h>


/** Opens a buffer for mlog, writes the initial log record and,
 if needed, the field lengths of an index.
 @return buffer, NULL if log mode MTR_LOG_NONE */
byte *mlog_open_and_write_index(
    mtr_t *mtr,                /*!< in: mtr */
    const byte *rec,           /*!< in: index record or page */
    const dict_index_t *index, /*!< in: record descriptor */
    mlog_id_t type,            /*!< in: log item type */
    ulint size)                /*!< in: requested buffer size in bytes
                               (if 0, calls mlog_close() and
                               returns NULL) */
{
#ifndef UNIV_HOTBACKUP
  byte *log_ptr;
  const byte *log_start;
  const byte *log_end;

  ut_ad(!!page_rec_is_comp(rec) == dict_table_is_comp(index->table));

  if (!page_rec_is_comp(rec)) {
    log_start = log_ptr = mlog_open(mtr, 11 + size);
    if (!log_ptr) {
      return (NULL); /* logging is disabled */
    }
    log_ptr = mlog_write_initial_log_record_fast(rec, type, log_ptr, mtr);
    log_end = log_ptr + 11 + size;
  } else {
    bool instant = index->has_instant_cols();
    ulint i;
    ulint n = dict_index_get_n_fields(index);
    ulint total = 11 + (instant ? 2 : 0) + size + (n + 2) * 2;
    ulint alloc = total;

    if (alloc > mtr_buf_t::MAX_DATA_SIZE) {
      alloc = mtr_buf_t::MAX_DATA_SIZE;
    }

    /* For spatial index, on non-leaf page, we just keep
    2 fields, MBR and page no. */
    if (dict_index_is_spatial(index) && !page_is_leaf(page_align(rec))) {
      n = DICT_INDEX_SPATIAL_NODEPTR_SIZE;
    }

    log_start = log_ptr = mlog_open(mtr, alloc);

    if (!log_ptr) {
      return (NULL); /* logging is disabled */
    }

    log_end = log_ptr + alloc;

    log_ptr = mlog_write_initial_log_record_fast(rec, type, log_ptr, mtr);

    if (instant) {
      /* Since the max columns could only be 1017,
      The leading bit is leveraged to indicate if the
      index is instant one and number of columns when
      first instant ADD COLUMN stored */
      mach_write_to_2(log_ptr, index->get_instant_fields() | 0x8000);
      log_ptr += 2;
    }
    mach_write_to_2(log_ptr, n);
    log_ptr += 2;

    if (page_is_leaf(page_align(rec))) {
      mach_write_to_2(log_ptr, dict_index_get_n_unique_in_tree(index));
    } else {
      mach_write_to_2(log_ptr, dict_index_get_n_unique_in_tree_nonleaf(index));
    }

    log_ptr += 2;

    for (i = 0; i < n; i++) {
      dict_field_t *field;
      const dict_col_t *col;
      ulint len;

      field = index->get_field(i);
      col = field->col;
      len = field->fixed_len;
      ut_ad(len < 0x7fff);
      if (len == 0 && (DATA_BIG_COL(col))) {
        /* variable-length field
        with maximum length > 255 */
        len = 0x7fff;
      }
      if (col->prtype & DATA_NOT_NULL) {
        len |= 0x8000;
      }
      if (log_ptr + 2 > log_end) {
        mlog_close(mtr, log_ptr);
        ut_a(total > (ulint)(log_ptr - log_start));
        total -= log_ptr - log_start;
        alloc = total;

        if (alloc > mtr_buf_t::MAX_DATA_SIZE) {
          alloc = mtr_buf_t::MAX_DATA_SIZE;
        }

        log_start = log_ptr = mlog_open(mtr, alloc);

        if (!log_ptr) {
          return (NULL); /* logging is disabled */
        }
        log_end = log_ptr + alloc;
      }
      mach_write_to_2(log_ptr, len);
      log_ptr += 2;
    }
  }
  if (size == 0) {
    mlog_close(mtr, log_ptr);
    log_ptr = NULL;
  } else if (log_ptr + size > log_end) {
    mlog_close(mtr, log_ptr);
    log_ptr = mlog_open(mtr, size);
  }
  return (log_ptr);
#else  /* !UNIV_HOTBACKUP */
  return (NULL);
#endif /* !UNIV_HOTBACKUP */
}
