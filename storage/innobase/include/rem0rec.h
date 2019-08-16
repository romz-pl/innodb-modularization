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

#include <innodb/bit/UT_BITS_IN_BYTES.h>
#include <innodb/data_types/dtuple_get_n_fields.h>
#include <innodb/record/flag.h>
#include <innodb/record/rec_get_instant_flag_new.h>
#include <innodb/record/rec_get_instant_flag_new_temp.h>
#include <innodb/record/rec_get_instant_offset.h>
#include <innodb/record/rec_get_n_extern_new.h>
#include <innodb/record/rec_get_n_fields.h>
#include <innodb/record/rec_get_n_fields_instant.h>
#include <innodb/record/rec_get_n_fields_old.h>
#include <innodb/record/rec_get_n_fields_old_raw.h>
#include <innodb/record/rec_get_offsets_func.h>
#include <innodb/record/rec_get_offsets_reverse.h>
#include <innodb/record/rec_get_status.h>
#include <innodb/record/rec_init_null_and_len_comp.h>
#include <innodb/record/rec_init_null_and_len_temp.h>
#include <innodb/record/rec_init_offsets.h>
#include <innodb/record/rec_init_offsets_comp_ordinary.h>
#include <innodb/record/rec_n_fields_is_sane.h>
#include <innodb/record/rec_offs_base.h>
#include <innodb/record/rec_offs_make_nth_extern.h>
#include <innodb/record/rec_offs_n_fields.h>
#include <innodb/record/rec_print.h>
#include <innodb/record/rec_print_mbr_rec.h>
#include <innodb/record/rec_print_new.h>
#include <innodb/record/rec_print_old.h>
#include <innodb/record/rec_get_offsets.h>
#include <innodb/record/rec_get_nth_field_old_instant.h>
#include <innodb/record/rec_get_nth_field_instant.h>
#include <innodb/record/rec_get_converted_size_comp_prefix_low.h>
#include <innodb/record/rec_get_converted_size_temp.h>
#include <innodb/record/rec_init_offsets_temp.h>
#include <innodb/record/rec_convert_dtuple_to_rec_comp.h>
#include <innodb/record/rec_convert_dtuple_to_temp.h>
#include <innodb/record/rec_copy_prefix_to_buf.h>
#include <innodb/record/rec_fold.h>
#include <innodb/record/rec_get_converted_size_comp.h>
#include <innodb/record/rec_convert_dtuple_to_rec_old.h>
#include <innodb/record/rec_convert_dtuple_to_rec_new.h>
#include <innodb/record/rec_convert_dtuple_to_rec.h>

#include "dict0boot.h"
#include "dict0dict.h"
#include "btr0types.h"

#include <ostream>
#include <sstream>






/** Determines the size of a data tuple prefix in ROW_FORMAT=COMPACT.
 @return total size */
ulint rec_get_converted_size_comp_prefix(
    const dict_index_t *index, /*!< in: record descriptor */
    const dfield_t *fields,    /*!< in: array of data fields */
    ulint n_fields,            /*!< in: number of data fields */
    ulint *extra)              /*!< out: extra size */
    MY_ATTRIBUTE((warn_unused_result));


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
