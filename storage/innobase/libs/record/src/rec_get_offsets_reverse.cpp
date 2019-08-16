#include <innodb/record/rec_get_offsets_reverse.h>

#include <innodb/dict_mem/dict_table_is_comp.h>
#include <innodb/dict_mem/dict_index_get_n_unique_in_tree_nonleaf.h>
#include <innodb/record/rec_offs_get_n_alloc.h>
#include <innodb/record/rec_offs_set_n_fields.h>
#include <innodb/dict_mem/dict_index_get_n_fields.h>
#include <innodb/bit/UT_BITS_IN_BYTES.h>
#include <innodb/record/rec_offs_base.h>
#include <innodb/record/rec_offs_n_fields.h>


/** The following function determines the offsets to each field
 in the record.  It can reuse a previously allocated array. */
void rec_get_offsets_reverse(
    const byte *extra,         /*!< in: the extra bytes of a
                               compact record in reverse order,
                               excluding the fixed-size
                               REC_N_NEW_EXTRA_BYTES */
    const dict_index_t *index, /*!< in: record descriptor */
    ulint node_ptr,            /*!< in: nonzero=node pointer,
                              0=leaf node */
    ulint *offsets)            /*!< in/out: array consisting of
                               offsets[0] allocated elements */
{
  ulint n;
  ulint i;
  ulint offs;
  ulint any_ext;
  const byte *nulls;
  const byte *lens;
  dict_field_t *field;
  ulint null_mask;
  ulint n_node_ptr_field;

  ut_ad(extra);
  ut_ad(index);
  ut_ad(offsets);
  ut_ad(dict_table_is_comp(index->table));

  if (UNIV_UNLIKELY(node_ptr)) {
    n_node_ptr_field = dict_index_get_n_unique_in_tree_nonleaf(index);
    n = n_node_ptr_field + 1;
  } else {
    n_node_ptr_field = ULINT_UNDEFINED;
    n = dict_index_get_n_fields(index);
  }

  ut_a(rec_offs_get_n_alloc(offsets) >= n + (1 + REC_OFFS_HEADER_SIZE));
  rec_offs_set_n_fields(offsets, n);

  nulls = extra;
  lens = nulls + UT_BITS_IN_BYTES(index->n_nullable);
  i = offs = 0;
  null_mask = 1;
  any_ext = 0;

  /* read the lengths of fields 0..n */
  do {
    ulint len;
    if (UNIV_UNLIKELY(i == n_node_ptr_field)) {
      len = offs += REC_NODE_PTR_SIZE;
      goto resolved;
    }

    field = index->get_field(i);
    if (!(field->col->prtype & DATA_NOT_NULL)) {
      /* nullable field => read the null flag */

      if (UNIV_UNLIKELY(!(byte)null_mask)) {
        nulls++;
        null_mask = 1;
      }

      if (*nulls & null_mask) {
        null_mask <<= 1;
        /* No length is stored for NULL fields.
        We do not advance offs, and we set
        the length to zero and enable the
        SQL NULL flag in offsets[]. */
        len = offs | REC_OFFS_SQL_NULL;
        goto resolved;
      }
      null_mask <<= 1;
    }

    if (UNIV_UNLIKELY(!field->fixed_len)) {
      /* Variable-length field: read the length */
      const dict_col_t *col = field->col;
      len = *lens++;
      /* If the maximum length of the field is up
      to 255 bytes, the actual length is always
      stored in one byte. If the maximum length is
      more than 255 bytes, the actual length is
      stored in one byte for 0..127.  The length
      will be encoded in two bytes when it is 128 or
      more, or when the field is stored externally. */
      if (DATA_BIG_COL(col)) {
        if (len & 0x80) {
          /* 1exxxxxxx xxxxxxxx */
          len <<= 8;
          len |= *lens++;

          offs += len & 0x3fff;
          if (UNIV_UNLIKELY(len & 0x4000)) {
            any_ext = REC_OFFS_EXTERNAL;
            len = offs | REC_OFFS_EXTERNAL;
          } else {
            len = offs;
          }

          goto resolved;
        }
      }

      len = offs += len;
    } else {
      len = offs += field->fixed_len;
    }
  resolved:
    rec_offs_base(offsets)[i + 1] = len;
  } while (++i < rec_offs_n_fields(offsets));

  ut_ad(lens >= extra);
  *rec_offs_base(offsets) =
      (lens - extra + REC_N_NEW_EXTRA_BYTES) | REC_OFFS_COMPACT | any_ext;
}


