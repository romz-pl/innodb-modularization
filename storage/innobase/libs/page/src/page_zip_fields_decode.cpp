#include <innodb/page/page_zip_fields_decode.h>

#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_mem/dict_mem_index_create.h>
#include <innodb/dict_mem/dict_mem_table_add_col.h>
#include <innodb/dict_mem/dict_mem_table_create.h>
#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/page/page_zip_fail.h>
#include <innodb/page/page_zip_fields_free.h>


void dict_index_add_col(dict_index_t *index, const dict_table_t *table,
                        dict_col_t *col, ulint prefix_len, bool is_ascending);

/** Read the index information for the compressed page.
@param[in]	buf		index information
@param[in]	end		end of buf
@param[in]	trx_id_col	NULL for non-leaf pages; for leaf pages,
                                pointer to where to store the position of the
                                trx_id column
@param[in]	is_spatial	is spatial index or not
@return own: dummy index describing the page, or NULL on error */
dict_index_t *page_zip_fields_decode(const byte *buf, const byte *end,
                                            ulint *trx_id_col,
                                            bool is_spatial) {
  const byte *b;
  ulint n;
  ulint i;
  ulint val;
  dict_table_t *table;
  dict_index_t *index;

  /* Determine the number of fields. */
  for (b = buf, n = 0; b < end; n++) {
    if (*b++ & 0x80) {
      b++; /* skip the second byte */
    }
  }

  n--; /* n_nullable or trx_id */

  if (UNIV_UNLIKELY(n > REC_MAX_N_FIELDS)) {
    page_zip_fail(("page_zip_fields_decode: n = %lu\n", (ulong)n));
    return (NULL);
  }

  if (UNIV_UNLIKELY(b > end)) {
    page_zip_fail(("page_zip_fields_decode: %p > %p\n", (const void *)b,
                   (const void *)end));
    return (NULL);
  }

  table = dict_mem_table_create("ZIP_DUMMY", DICT_HDR_SPACE, n, 0,
                                DICT_TF_COMPACT, 0);
  index = dict_mem_index_create("ZIP_DUMMY", "ZIP_DUMMY", DICT_HDR_SPACE, 0, n);
  index->table = table;
  index->n_uniq = n;
  /* avoid ut_ad(index->cached) in dict_index_get_n_unique_in_tree */
  index->cached = TRUE;

  /* Initialize the fields. */
  for (b = buf, i = 0; i < n; i++) {
    ulint mtype;
    ulint len;

    val = *b++;

    if (UNIV_UNLIKELY(val & 0x80)) {
      /* fixed length > 62 bytes */
      val = (val & 0x7f) << 8 | *b++;
      len = val >> 1;
      mtype = DATA_FIXBINARY;
    } else if (UNIV_UNLIKELY(val >= 126)) {
      /* variable length with max > 255 bytes */
      len = 0x7fff;
      mtype = DATA_BINARY;
    } else if (val <= 1) {
      /* variable length with max <= 255 bytes */
      len = 0;
      mtype = DATA_BINARY;
    } else {
      /* fixed length < 62 bytes */
      len = val >> 1;
      mtype = DATA_FIXBINARY;
    }

    dict_mem_table_add_col(table, NULL, NULL, mtype,
                           val & 1 ? DATA_NOT_NULL : 0, len);

    /* The is_ascending flag does not matter during decompression,
    because we do not compare for "less than" or "greater than" */
    dict_index_add_col(index, table, table->get_col(i), 0, true);
  }

  val = *b++;
  if (UNIV_UNLIKELY(val & 0x80)) {
    val = (val & 0x7f) << 8 | *b++;
  }

  /* Decode the position of the trx_id column. */
  if (trx_id_col) {
    if (!val) {
      val = ULINT_UNDEFINED;
    } else if (UNIV_UNLIKELY(val >= n)) {
      page_zip_fields_free(index);
      index = NULL;
    } else {
      index->type = DICT_CLUSTERED;
    }

    *trx_id_col = val;
  } else {
    /* Decode the number of nullable fields. */
    if (UNIV_UNLIKELY(index->n_nullable > val)) {
      page_zip_fields_free(index);
      index = NULL;
    } else {
      index->n_nullable = val;
    }
  }

  ut_ad(b == end);

  if (is_spatial) {
    index->type |= DICT_SPATIAL;
  }

  index->n_instant_nullable = index->n_nullable;
  index->instant_cols =
      (index->is_clustered() && index->table->has_instant_cols());

  return (index);
}
