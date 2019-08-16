/*****************************************************************************

Copyright (c) 1994, 2018, Oracle and/or its affiliates. All Rights Reserved.

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

/** @file rem/rem0rec.cc
 Record manager

 Created 5/30/1994 Heikki Tuuri
 *************************************************************************/

#include <innodb/data_types/dfield_set_null.h>
#include <innodb/data_types/dfield_set_data.h>
#include <innodb/data_types/dfield_is_ext.h>
#include <innodb/data_types/dfield_is_null.h>
#include <innodb/memory/mem_heap_dup.h>
#include <innodb/data_types/dtuple_set_info_bits.h>
#include <innodb/data_types/dtuple_get_nth_field.h>
#include <innodb/data_types/dtype_get_sql_null_size.h>
#include <innodb/data_types/dtuple_get_nth_v_field.h>
#include <innodb/data_types/dtuple_get_n_v_fields.h>
#include <innodb/allocator/ut_malloc_nokey.h>
#include <innodb/data_types/dtype_get_mtype.h>
#include <innodb/data_types/dtype_get_len.h>
#include <innodb/print/ut_print_buf.h>
#include <innodb/print/ut_print_buf_hex.h>
#include <innodb/record/rec_set_n_fields_old.h>
#include <innodb/record/rec_set_info_bits_old.h>
#include <innodb/record/rec_set_1byte_offs_flag.h>
#include <innodb/record/data_write_sql_null.h>
#include <innodb/record/rec_1_set_field_end_info.h>
#include <innodb/record/rec_2_set_field_end_info.h>
#include <innodb/record/rec_set_info_and_status_bits.h>
#include <innodb/record/rec_set_instant_flag_new.h>
#include <innodb/record/rec_offs_init.h>
#include <innodb/record/rec_get_1byte_offs_flag.h>
#include <innodb/record/rec_get_field_start_offs.h>
#include <innodb/record/rec_get_nth_field_size.h>
#include <innodb/record/rec_validate_old.h>
#include <innodb/record/rec_offs_nth_default.h>
#include <innodb/record/rec_offs_comp.h>
#include <innodb/record/rec_validate.h>
#include <innodb/record/rec_copy_prefix_to_buf_old.h>
#include <innodb/record/rec_print_comp.h>
#include <innodb/record/rec_print_mbr_old.h>
#include <innodb/data_types/dtype_get_prtype.h>

#include "rem0rec.h"

#include <sys/types.h>
#ifndef UNIV_HOTBACKUP

#include "fts0fts.h"
#endif /* !UNIV_HOTBACKUP */
#include "gis0geo.h"
#include <innodb/machine/data.h>
#include "mtr0log.h"

#include "page0page.h"
#include "trx0sys.h"


#include "my_dbug.h"

/*			PHYSICAL RECORD (OLD STYLE)
                        ===========================

The physical record, which is the data type of all the records
found in index pages of the database, has the following format
(lower addresses and more significant bits inside a byte are below
represented on a higher text line):

| offset of the end of the last field of data, the most significant
  bit is set to 1 if and only if the field is SQL-null,
  if the offset is 2-byte, then the second most significant
  bit is set to 1 if the field is stored on another page:
  mostly this will occur in the case of big BLOB fields |
...
| offset of the end of the first field of data + the SQL-null bit |
| 4 bits used to delete mark a record, and mark a predefined
  minimum record in alphabetical order |
| 4 bits giving the number of records owned by this record
  (this term is explained in page0page.h) |
| 13 bits giving the order number of this record in the
  heap of the index page |
| 10 bits giving the number of fields in this record |
| 1 bit which is set to 1 if the offsets above are given in
  one byte format, 0 if in two byte format |
| two bytes giving an absolute pointer to the next record in the page |
ORIGIN of the record
| first field of data |
...
| last field of data |

The origin of the record is the start address of the first field
of data. The offsets are given relative to the origin.
The offsets of the data fields are stored in an inverted
order because then the offset of the first fields are near the
origin, giving maybe a better processor cache hit rate in searches.

The offsets of the data fields are given as one-byte
(if there are less than 127 bytes of data in the record)
or two-byte unsigned integers. The most significant bit
is not part of the offset, instead it indicates the SQL-null
if the bit is set to 1. */

/*			PHYSICAL RECORD (NEW STYLE)
                        ===========================

The physical record, which is the data type of all the records
found in index pages of the database, has the following format
(lower addresses and more significant bits inside a byte are below
represented on a higher text line):

| length of the last non-null variable-length field of data:
  if the maximum length is 255, one byte; otherwise,
  0xxxxxxx (one byte, length=0..127), or 1exxxxxxxxxxxxxx (two bytes,
  length=128..16383, extern storage flag) |
...
| length of first variable-length field of data |
| SQL-null flags (1 bit per nullable field), padded to full bytes |
| 1 or 2 bytes to indicate number of fields in the record if the table
  where the record resides has undergone an instant ADD COLUMN
  before this record gets inserted; If no instant ADD COLUMN ever
  happened, here should be no byte; So parsing this optional number
  requires the index or table information |
| 4 bits used to delete mark a record, and mark a predefined
  minimum record in alphabetical order |
| 4 bits giving the number of records owned by this record
  (this term is explained in page0page.h) |
| 13 bits giving the order number of this record in the
  heap of the index page |
| 3 bits record type: 000=conventional, 001=node pointer (inside B-tree),
  010=infimum, 011=supremum, 1xx=reserved |
| two bytes giving a relative pointer to the next record in the page |
ORIGIN of the record
| first field of data |
...
| last field of data |

The origin of the record is the start address of the first field
of data. The offsets are given relative to the origin.
The offsets of the data fields are stored in an inverted
order because then the offset of the first fields are near the
origin, giving maybe a better processor cache hit rate in searches.

The offsets of the data fields are given as one-byte
(if there are less than 127 bytes of data in the record)
or two-byte unsigned integers. The most significant bit
is not part of the offset, instead it indicates the SQL-null
if the bit is set to 1. */

/* CANONICAL COORDINATES. A record can be seen as a single
string of 'characters' in the following way: catenate the bytes
in each field, in the order of fields. An SQL-null field
is taken to be an empty sequence of bytes. Then after
the position of each field insert in the string
the 'character' <FIELD-END>, except that after an SQL-null field
insert <NULL-FIELD-END>. Now the ordinal position of each
byte in this canonical string is its canonical coordinate.
So, for the record ("AA", SQL-NULL, "BB", ""), the canonical
string is "AA<FIELD_END><NULL-FIELD-END>BB<FIELD-END><FIELD-END>".
We identify prefixes (= initial segments) of a record
with prefixes of the canonical string. The canonical
length of the prefix is the length of the corresponding
prefix of the canonical string. The canonical length of
a record is the length of its canonical string.

For example, the maximal common prefix of records
("AA", SQL-NULL, "BB", "C") and ("AA", SQL-NULL, "B", "C")
is "AA<FIELD-END><NULL-FIELD-END>B", and its canonical
length is 5.

A complete-field prefix of a record is a prefix which ends at the
end of some field (containing also <FIELD-END>).
A record is a complete-field prefix of another record, if
the corresponding canonical strings have the same property. */





/** Determines the size of a data tuple prefix in ROW_FORMAT=COMPACT.
 @return total size */
ulint rec_get_converted_size_comp_prefix(
    const dict_index_t *index, /*!< in: record descriptor */
    const dfield_t *fields,    /*!< in: array of data fields */
    ulint n_fields,            /*!< in: number of data fields */
    ulint *extra)              /*!< out: extra size */
{
  ut_ad(dict_table_is_comp(index->table));
  return (rec_get_converted_size_comp_prefix_low(index, fields, n_fields, NULL,
                                                 extra, nullptr, false));
}





#ifndef UNIV_HOTBACKUP






/** Copies the first n fields of a physical record to a data tuple. The fields
 are copied to the memory heap. */
void rec_copy_prefix_to_dtuple(
    dtuple_t *tuple,           /*!< out: data tuple */
    const rec_t *rec,          /*!< in: physical record */
    const dict_index_t *index, /*!< in: record descriptor */
    ulint n_fields,            /*!< in: number of fields
                               to copy */
    mem_heap_t *heap)          /*!< in: memory heap */
{
  ulint i;
  ulint offsets_[REC_OFFS_NORMAL_SIZE];
  ulint *offsets = offsets_;
  rec_offs_init(offsets_);

  offsets = rec_get_offsets(rec, index, offsets, n_fields, &heap);

  ut_ad(rec_validate(rec, offsets));
  ut_ad(dtuple_check_typed(tuple));

  dtuple_set_info_bits(
      tuple, rec_get_info_bits(rec, dict_table_is_comp(index->table)));

  for (i = 0; i < n_fields; i++) {
    dfield_t *field;
    const byte *data;
    ulint len;

    field = dtuple_get_nth_field(tuple, i);
    data = rec_get_nth_field_instant(rec, offsets, i, index, &len);

    if (len != UNIV_SQL_NULL) {
      dfield_set_data(field, mem_heap_dup(heap, data, len), len);
      ut_ad(!rec_offs_nth_extern(offsets, i));
    } else {
      dfield_set_null(field);
    }
  }
}

#endif /* UNIV_HOTBACKUP */



#ifndef UNIV_HOTBACKUP




/** Prints a physical record. */
void rec_print(FILE *file,                /*!< in: file where to print */
               const rec_t *rec,          /*!< in: physical record */
               const dict_index_t *index) /*!< in: record descriptor */
{
  ut_ad(index);

  if (!dict_table_is_comp(index->table)) {
    rec_print_old(file, rec);
    return;
  } else {
    mem_heap_t *heap = NULL;
    ulint offsets_[REC_OFFS_NORMAL_SIZE];
    rec_offs_init(offsets_);

    rec_print_new(
        file, rec,
        rec_get_offsets(rec, index, offsets_, ULINT_UNDEFINED, &heap));
    if (UNIV_LIKELY_NULL(heap)) {
      mem_heap_free(heap);
    }
  }
}


/** Display a record.
@param[in,out]	o	output stream
@param[in]	r	record to display
@return	the output stream */
std::ostream &operator<<(std::ostream &o, const rec_index_print &r) {
  mem_heap_t *heap = NULL;
  ulint *offsets =
      rec_get_offsets(r.m_rec, r.m_index, NULL, ULINT_UNDEFINED, &heap);
  rec_print(o, r.m_rec, rec_get_info_bits(r.m_rec, rec_offs_comp(offsets)),
            offsets);
  mem_heap_free(heap);
  return (o);
}



/** Reads the DB_TRX_ID of a clustered index record.
 @return the value of DB_TRX_ID */
trx_id_t rec_get_trx_id(const rec_t *rec,          /*!< in: record */
                        const dict_index_t *index) /*!< in: clustered index */
{
  ulint trx_id_col = index->get_sys_col_pos(DATA_TRX_ID);
  const byte *trx_id;
  ulint len;
  mem_heap_t *heap = NULL;
  ulint offsets_[REC_OFFS_NORMAL_SIZE];
  ulint *offsets = offsets_;
  rec_offs_init(offsets_);

  ut_ad(index->is_clustered());
  ut_ad(trx_id_col > 0);
  ut_ad(trx_id_col != ULINT_UNDEFINED);

#ifdef UNIV_DEBUG
  const page_t *page = page_align(rec);
  if (fil_page_index_page_check(page)) {
    ut_ad(mach_read_from_8(page + PAGE_HEADER + PAGE_INDEX_ID) == index->id);
  }
#endif /* UNIV_DEBUG */

  offsets = rec_get_offsets(rec, index, offsets, trx_id_col + 1, &heap);

  trx_id = rec_get_nth_field(rec, offsets, trx_id_col, &len);

  ut_ad(len == DATA_TRX_ID_LEN);

  if (heap) {
    mem_heap_free(heap);
  }

  return (trx_read_trx_id(trx_id));
}
#endif /* !UNIV_HOTBACKUP */


