#include <innodb/record/rec_copy_prefix_to_buf.h>

#include <innodb/allocator/ut_free.h>
#include <innodb/allocator/ut_malloc_nokey.h>
#include <innodb/dict_mem/dict_index_is_spatial.h>
#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_mem/dict_table_is_comp.h>
#include <innodb/record/flag.h>
#include <innodb/record/rec_copy_prefix_to_buf_old.h>
#include <innodb/record/rec_get_field_start_offs.h>
#include <innodb/record/rec_get_status.h>
#include <innodb/record/rec_init_null_and_len_comp.h>

rec_t *rec_copy_prefix_to_buf(const rec_t *rec, const dict_index_t *index,
                              ulint n_fields, byte **buf, size_t *buf_size) {
  const byte *nulls;
  const byte *lens;
  uint16_t n_null;
  ulint i;
  ulint prefix_len;
  ulint null_mask;
  ulint status;
  bool is_rtr_node_ptr = false;

  UNIV_PREFETCH_RW(*buf);

  if (!dict_table_is_comp(index->table)) {
    ut_ad(rec_validate_old(rec));
    return (rec_copy_prefix_to_buf_old(
        rec, n_fields, rec_get_field_start_offs(rec, n_fields), buf, buf_size));
  }

  status = rec_get_status(rec);

  switch (status) {
    case REC_STATUS_ORDINARY:
      ut_ad(n_fields <= dict_index_get_n_fields(index));
      break;
    case REC_STATUS_NODE_PTR:
      /* For R-tree, we need to copy the child page number field. */
      if (dict_index_is_spatial(index)) {
        ut_ad(n_fields == DICT_INDEX_SPATIAL_NODEPTR_SIZE + 1);
        is_rtr_node_ptr = true;
      } else {
        /* it doesn't make sense to copy the child page number
        field */
        ut_ad(n_fields <= dict_index_get_n_unique_in_tree_nonleaf(index));
      }
      break;
    case REC_STATUS_INFIMUM:
    case REC_STATUS_SUPREMUM:
      /* infimum or supremum record: no sense to copy anything */
    default:
      ut_error;
      return (NULL);
  }

  ut_d(uint16_t non_default_fields =)
      rec_init_null_and_len_comp(rec, index, &nulls, &lens, &n_null);
  ut_ad(!rec_get_instant_flag_new(rec) || n_fields <= non_default_fields);

  UNIV_PREFETCH_R(lens);
  prefix_len = 0;
  null_mask = 1;

  /* read the lengths of fields 0..n */
  for (i = 0; i < n_fields; i++) {
    const dict_field_t *field;
    const dict_col_t *col;

    field = index->get_field(i);
    col = field->col;

    if (!(col->prtype & DATA_NOT_NULL)) {
      /* nullable field => read the null flag */
      if (UNIV_UNLIKELY(!(byte)null_mask)) {
        nulls--;
        null_mask = 1;
      }

      if (*nulls & null_mask) {
        null_mask <<= 1;
        continue;
      }

      null_mask <<= 1;
    }

    if (is_rtr_node_ptr && i == 1) {
      /* For rtree node ptr rec, we need to
      copy the page no field with 4 bytes len. */
      prefix_len += 4;
    } else if (field->fixed_len) {
      prefix_len += field->fixed_len;
    } else {
      ulint len = *lens--;
      /* If the maximum length of the column is up
      to 255 bytes, the actual length is always
      stored in one byte. If the maximum length is
      more than 255 bytes, the actual length is
      stored in one byte for 0..127.  The length
      will be encoded in two bytes when it is 128 or
      more, or when the column is stored externally. */
      if (DATA_BIG_COL(col)) {
        if (len & 0x80) {
          /* 1exxxxxx */
          len &= 0x3f;
          len <<= 8;
          len |= *lens--;
          UNIV_PREFETCH_R(lens);
        }
      }
      prefix_len += len;
    }
  }

  UNIV_PREFETCH_R(rec + prefix_len);

  prefix_len += rec - (lens + 1);

  if ((*buf == NULL) || (*buf_size < prefix_len)) {
    ut_free(*buf);
    *buf_size = prefix_len;
    *buf = static_cast<byte *>(ut_malloc_nokey(prefix_len));
  }

  memcpy(*buf, lens + 1, prefix_len);

  return (*buf + (rec - (lens + 1)));
}
