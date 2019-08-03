/*****************************************************************************

Copyright (c) 1994, 2019, Oracle and/or its affiliates. All Rights Reserved.

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

/** @file include/rem0rec.h
 Record manager

 Created 5/30/1994 Heikki Tuuri
 *************************************************************************/

#ifndef rem0rec_h
#define rem0rec_h

#include <innodb/univ/univ.h>
#include <innodb/record/rec_print_old.h>
#include <innodb/record/rec_offs_make_nth_extern.h>
#include <innodb/record/rec_print_mbr_rec.h>
#include <innodb/record/rec_print_new.h>
#include <innodb/record/rec_print.h>

#include <ostream>
#include <sstream>

#include "data0data.h"
#include "mtr0types.h"
#include "rem/rec.h"

#include "trx0types.h"


/** The following function is used to set the number of owned records.
@param[in,out]	rec		new-style physical record
@param[in,out]	page_zip	compressed page, or NULL
@param[in]	n_owned		the number of owned */
UNIV_INLINE
void rec_set_n_owned_new(rec_t *rec, page_zip_des_t *page_zip, ulint n_owned);


/** The following function is used to set the deleted bit.
@param[in,out]	rec		new-style physical record
@param[in,out]	page_zip	compressed page, or NULL
@param[in]	flag		nonzero if delete marked */
UNIV_INLINE
void rec_set_deleted_flag_new(rec_t *rec, page_zip_des_t *page_zip, ulint flag);

/** Determine how many of the first n columns in a compact
 physical record are stored externally.
 @return number of externally stored columns */
ulint rec_get_n_extern_new(
    const rec_t *rec,          /*!< in: compact physical record */
    const dict_index_t *index, /*!< in: record descriptor */
    ulint n)                   /*!< in: number of columns to scan */
    MY_ATTRIBUTE((warn_unused_result));

#ifdef UNIV_DEBUG
#define rec_get_offsets(rec, index, offsets, n, heap) \
  rec_get_offsets_func(rec, index, offsets, n, __FILE__, __LINE__, heap)
#else /* UNIV_DEBUG */
#define rec_get_offsets(rec, index, offsets, n, heap) \
  rec_get_offsets_func(rec, index, offsets, n, heap)
#endif /* UNIV_DEBUG */



/** Gets the value of the specified field in the record in old style.
This is only used for record from instant index, which is clustered
index and has some instantly added columns.
@param[in]	rec	physical record
@param[in]	n	index of the field
@param[in]	index	clustered index where the record resides
@param[out]	len	length of the field, UNIV_SQL if SQL null
@return value of the field, could be either pointer to rec or default value */
UNIV_INLINE
const byte *rec_get_nth_field_old_instant(const rec_t *rec, uint16_t n,
                                          const dict_index_t *index,
                                          ulint *len);


/** Gets the value of the specified field in the record.
This is only used when there is possibility that the record comes from the
clustered index, which has some instantly added columns.
@param[in]	rec	physical record
@param[in]	offsets	array returned by rec_get_offsets()
@param[in]	n	index of the field
@param[in]	index	clustered index where the record resides, or nullptr
                        if the record doesn't have instantly added columns
                        for sure
@param[out]	len	length of the field, UNIV_SQL_NULL if SQL null
@return	value of the field, could be either pointer to rec or default value */
UNIV_INLINE
const byte *rec_get_nth_field_instant(const rec_t *rec, const ulint *offsets,
                                      ulint n, const dict_index_t *index,
                                      ulint *len);










#ifndef UNIV_HOTBACKUP
/** Determines the size of a data tuple prefix in a temporary file.
 @return total size */
ulint rec_get_converted_size_temp(
    const dict_index_t *index, /*!< in: record descriptor */
    const dfield_t *fields,    /*!< in: array of data fields */
    ulint n_fields,            /*!< in: number of data fields */
    const dtuple_t *v_entry,   /*!< in: dtuple contains virtual column
                               data */
    ulint *extra)              /*!< out: extra size */
    MY_ATTRIBUTE((warn_unused_result));

/** Determine the offset to each field in temporary file.
 @see rec_convert_dtuple_to_temp() */
void rec_init_offsets_temp(
    const rec_t *rec,          /*!< in: temporary file record */
    const dict_index_t *index, /*!< in: record descriptor */
    ulint *offsets);           /*!< in/out: array of offsets;
                              in: n=rec_offs_n_fields(offsets) */

/** Builds a temporary file record out of a data tuple.
 @see rec_init_offsets_temp() */
void rec_convert_dtuple_to_temp(
    rec_t *rec,                /*!< out: record */
    const dict_index_t *index, /*!< in: record descriptor */
    const dfield_t *fields,    /*!< in: array of data fields */
    ulint n_fields,            /*!< in: number of fields */
    const dtuple_t *v_entry);  /*!< in: dtuple contains
                               virtual column data */

/** Copies the first n fields of a physical record to a new physical record in
 a buffer.
 @return own: copied record */
rec_t *rec_copy_prefix_to_buf(
    const rec_t *rec,          /*!< in: physical record */
    const dict_index_t *index, /*!< in: record descriptor */
    ulint n_fields,            /*!< in: number of fields
                               to copy */
    byte **buf,                /*!< in/out: memory buffer
                               for the copied prefix,
                               or NULL */
    size_t *buf_size           /*!< in/out: buffer size */
);
/** Compute a hash value of a prefix of a leaf page record.
@param[in]	rec		leaf page record
@param[in]	offsets		rec_get_offsets(rec)
@param[in]	n_fields	number of complete fields to fold
@param[in]	n_bytes		number of bytes to fold in the last field
@param[in]	fold		fold value of the index identifier
@param[in]	index		index where the record resides
@return the folded value */
UNIV_INLINE
ulint rec_fold(const rec_t *rec, const ulint *offsets, ulint n_fields,
               ulint n_bytes, ulint fold, const dict_index_t *index)
    MY_ATTRIBUTE((warn_unused_result));
#endif /* !UNIV_HOTBACKUP */
/** Builds a physical record out of a data tuple and
 stores it into the given buffer.
 @return pointer to the origin of physical record */
rec_t *rec_convert_dtuple_to_rec(
    byte *buf,                 /*!< in: start address of the
                               physical record */
    const dict_index_t *index, /*!< in: record descriptor */
    const dtuple_t *dtuple,    /*!< in: data tuple */
    ulint n_ext)               /*!< in: number of
                               externally stored columns */
    MY_ATTRIBUTE((warn_unused_result));

/** Determines the size of a data tuple prefix in ROW_FORMAT=COMPACT.
 @return total size */
ulint rec_get_converted_size_comp_prefix(
    const dict_index_t *index, /*!< in: record descriptor */
    const dfield_t *fields,    /*!< in: array of data fields */
    ulint n_fields,            /*!< in: number of data fields */
    ulint *extra)              /*!< out: extra size */
    MY_ATTRIBUTE((warn_unused_result));

/** Determines the size of a data tuple in ROW_FORMAT=COMPACT.
 @return total size */
ulint rec_get_converted_size_comp(
    const dict_index_t *index, /*!< in: record descriptor;
                               dict_table_is_comp() is
                               assumed to hold, even if
                               it does not */
    ulint status,              /*!< in: status bits of the record */
    const dfield_t *fields,    /*!< in: array of data fields */
    ulint n_fields,            /*!< in: number of data fields */
    ulint *extra);             /*!< out: extra size */
/** The following function returns the size of a data tuple when converted to
 a physical record.
 @return size */
UNIV_INLINE
ulint rec_get_converted_size(
    const dict_index_t *index, /*!< in: record descriptor */
    const dtuple_t *dtuple,    /*!< in: data tuple */
    ulint n_ext)               /*!< in: number of externally stored columns */
    MY_ATTRIBUTE((warn_unused_result));
#ifndef UNIV_HOTBACKUP
/** Copies the first n fields of a physical record to a data tuple.
 The fields are copied to the memory heap. */
void rec_copy_prefix_to_dtuple(
    dtuple_t *tuple,           /*!< out: data tuple */
    const rec_t *rec,          /*!< in: physical record */
    const dict_index_t *index, /*!< in: record descriptor */
    ulint n_fields,            /*!< in: number of fields
                               to copy */
    mem_heap_t *heap);         /*!< in: memory heap */
#endif                         /* !UNIV_HOTBACKUP */

/** Get the length of the number of fields for any new style record.
@param[in]	n_fields	number of fields in the record
@return	length of specified number of fields */
UNIV_INLINE
uint8_t rec_get_n_fields_length(ulint n_fields);

/** Set the number of fields for one new style leaf page record.
This is only needed for table after instant ADD COLUMN.
@param[in,out]	rec		leaf page record
@param[in]	n_fields	number of fields in the record
@return the length of the n_fields occupies */
UNIV_INLINE
uint8_t rec_set_n_fields(rec_t *rec, ulint n_fields);



#ifndef UNIV_HOTBACKUP


/** Prints a physical record. */
void rec_print(FILE *file,                 /*!< in: file where to print */
               const rec_t *rec,           /*!< in: physical record */
               const dict_index_t *index); /*!< in: record descriptor */


/** Wrapper for pretty-printing a record */
struct rec_index_print {
  /** Constructor */
  rec_index_print(const rec_t *rec, const dict_index_t *index)
      : m_rec(rec), m_index(index) {}

  /** Record */
  const rec_t *m_rec;
  /** Index */
  const dict_index_t *m_index;
};

/** Display a record.
@param[in,out]	o	output stream
@param[in]	r	record to display
@return	the output stream */
std::ostream &operator<<(std::ostream &o, const rec_index_print &r);



#ifdef UNIV_DEBUG
/** Pretty-printer of records and tuples */
class rec_printer : public std::ostringstream {
 public:
  /** Construct a pretty-printed record.
  @param rec	record with header
  @param offsets	rec_get_offsets(rec, ...) */
  rec_printer(const rec_t *rec, const ulint *offsets) : std::ostringstream() {
    rec_print(*this, rec, rec_get_info_bits(rec, rec_offs_comp(offsets)),
              offsets);
  }

  /** Construct a pretty-printed record.
  @param rec record, possibly lacking header
  @param info rec_get_info_bits(rec)
  @param offsets rec_get_offsets(rec, ...) */
  rec_printer(const rec_t *rec, ulint info, const ulint *offsets)
      : std::ostringstream() {
    rec_print(*this, rec, info, offsets);
  }

  /** Construct a pretty-printed tuple.
  @param tuple	data tuple */
  rec_printer(const dtuple_t *tuple) : std::ostringstream() {
    dtuple_print(*this, tuple);
  }

  /** Construct a pretty-printed tuple.
  @param field	array of data tuple fields
  @param n	number of fields */
  rec_printer(const dfield_t *field, ulint n) : std::ostringstream() {
    dfield_print(*this, field, n);
  }

  /** Destructor */
  virtual ~rec_printer() {}

 private:
  /** Copy constructor */
  rec_printer(const rec_printer &other);
  /** Assignment operator */
  rec_printer &operator=(const rec_printer &other);
};
#endif /* UNIV_DEBUG */

/** Reads the DB_TRX_ID of a clustered index record.
 @return the value of DB_TRX_ID */
trx_id_t rec_get_trx_id(const rec_t *rec,          /*!< in: record */
                        const dict_index_t *index) /*!< in: clustered index */
    MY_ATTRIBUTE((warn_unused_result));
#endif /* UNIV_HOTBACKUP */



#include "rem0rec.ic"

#endif
