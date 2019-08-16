#include <innodb/record/rec_init_offsets.h>

#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_mem/dict_table_is_comp.h>
#include <innodb/dict_mem/dict_index_get_n_unique_in_tree_nonleaf.h>
#include <innodb/record/rec_offs_make_valid.h>
#include <innodb/record/rec_get_status.h>
#include <innodb/record/rec_offs_base.h>
#include <innodb/record/rec_init_offsets_comp_ordinary.h>
#include <innodb/record/rec_get_instant_flag_new.h>
#include <innodb/record/rec_offs_n_fields.h>
#include <innodb/record/rec_get_1byte_offs_flag.h>
#include <innodb/record/rec_get_n_fields_old_raw.h>
#include <innodb/record/rec_1_get_field_end_info.h>
#include <innodb/record/rec_2_get_field_end_info.h>

/** The following function determines the offsets to each field in the
 record.	 The offsets are written to a previously allocated array of
 ulint, where rec_offs_n_fields(offsets) has been initialized to the
 number of fields in the record.	 The rest of the array will be
 initialized by this function.  rec_offs_base(offsets)[0] will be set
 to the extra size (if REC_OFFS_COMPACT is set, the record is in the
 new format; if REC_OFFS_EXTERNAL is set, the record contains externally
 stored columns), and rec_offs_base(offsets)[1..n_fields] will be set to
 offsets past the end of fields 0..n_fields, or to the beginning of
 fields 1..n_fields+1.  When the high-order bit of the offset at [i+1]
 is set (REC_OFFS_SQL_NULL), the field i is NULL.  When the second
 high-order bit of the offset at [i+1] is set (REC_OFFS_EXTERNAL), the
 field i is being stored externally. */
void rec_init_offsets(const rec_t *rec,          /*!< in: physical record */
                      const dict_index_t *index, /*!< in: record descriptor */
                      ulint *offsets)            /*!< in/out: array of offsets;
                                                 in: n=rec_offs_n_fields(offsets) */
{
  ulint i = 0;
  ulint offs;

  rec_offs_make_valid(rec, index, offsets);

  if (dict_table_is_comp(index->table)) {
    const byte *nulls;
    const byte *lens;
    dict_field_t *field;
    ulint null_mask;
    ulint status = rec_get_status(rec);
    ulint n_node_ptr_field = ULINT_UNDEFINED;

    switch (UNIV_EXPECT(status, REC_STATUS_ORDINARY)) {
      case REC_STATUS_INFIMUM:
      case REC_STATUS_SUPREMUM:
        /* the field is 8 bytes long */
        rec_offs_base(offsets)[0] = REC_N_NEW_EXTRA_BYTES | REC_OFFS_COMPACT;
        rec_offs_base(offsets)[1] = 8;
        return;
      case REC_STATUS_NODE_PTR:
        n_node_ptr_field = dict_index_get_n_unique_in_tree_nonleaf(index);
        break;
      case REC_STATUS_ORDINARY:
        rec_init_offsets_comp_ordinary(rec, false, index, offsets);
        return;
    }

    ut_ad(!rec_get_instant_flag_new(rec));

    nulls = rec - (REC_N_NEW_EXTRA_BYTES + 1);
    lens = nulls - UT_BITS_IN_BYTES(index->n_instant_nullable);
    offs = 0;
    null_mask = 1;

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
          nulls--;
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
        const dict_col_t *col = field->col;
        /* DATA_POINT should always be a fixed
        length column. */
        ut_ad(col->mtype != DATA_POINT);
        /* Variable-length field: read the length */
        len = *lens--;
        /* If the maximum length of the field
        is up to 255 bytes, the actual length
        is always stored in one byte. If the
        maximum length is more than 255 bytes,
        the actual length is stored in one
        byte for 0..127.  The length will be
        encoded in two bytes when it is 128 or
        more, or when the field is stored
        externally. */
        if (DATA_BIG_COL(col)) {
          if (len & 0x80) {
            /* 1exxxxxxx xxxxxxxx */

            len <<= 8;
            len |= *lens--;

            /* B-tree node pointers
            must not contain externally
            stored columns.  Thus
            the "e" flag must be 0. */
            ut_a(!(len & 0x4000));
            offs += len & 0x3fff;
            len = offs;

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

    *rec_offs_base(offsets) = (rec - (lens + 1)) | REC_OFFS_COMPACT;
  } else {
    /* Old-style record: determine extra size and end offsets */
    offs = REC_N_OLD_EXTRA_BYTES;
    if (rec_get_1byte_offs_flag(rec)) {
      offs += rec_get_n_fields_old_raw(rec);
      *rec_offs_base(offsets) = offs;
      /* Determine offsets to fields */
      do {
        if (index->has_instant_cols() && i >= rec_get_n_fields_old_raw(rec)) {
          offs &= ~REC_OFFS_SQL_NULL;
          offs = rec_get_instant_offset(index, i, offs);
        } else {
          offs = rec_1_get_field_end_info(rec, i);
        }

        if (offs & REC_1BYTE_SQL_NULL_MASK) {
          offs &= ~REC_1BYTE_SQL_NULL_MASK;
          offs |= REC_OFFS_SQL_NULL;
        }

        ut_ad(i < rec_get_n_fields_old_raw(rec) || (offs & REC_OFFS_SQL_NULL) ||
              (offs & REC_OFFS_DEFAULT));
        rec_offs_base(offsets)[1 + i] = offs;
      } while (++i < rec_offs_n_fields(offsets));
    } else {
      offs += 2 * rec_get_n_fields_old_raw(rec);
      *rec_offs_base(offsets) = offs;
      /* Determine offsets to fields */
      do {
        if (index->has_instant_cols() && i >= rec_get_n_fields_old_raw(rec)) {
          offs &= ~(REC_OFFS_SQL_NULL | REC_OFFS_EXTERNAL);
          offs = rec_get_instant_offset(index, i, offs);
        } else {
          offs = rec_2_get_field_end_info(rec, i);
        }

        if (offs & REC_2BYTE_SQL_NULL_MASK) {
          offs &= ~REC_2BYTE_SQL_NULL_MASK;
          offs |= REC_OFFS_SQL_NULL;
        }
        if (offs & REC_2BYTE_EXTERN_MASK) {
          offs &= ~REC_2BYTE_EXTERN_MASK;
          offs |= REC_OFFS_EXTERNAL;
          *rec_offs_base(offsets) |= REC_OFFS_EXTERNAL;
        }

        ut_ad(i < rec_get_n_fields_old_raw(rec) || (offs & REC_OFFS_SQL_NULL) ||
              (offs & REC_OFFS_DEFAULT));
        rec_offs_base(offsets)[1 + i] = offs;
      } while (++i < rec_offs_n_fields(offsets));
    }
  }
}
