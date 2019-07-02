#pragma once

#include <innodb/univ/univ.h>

/* We define the physical record simply as an array of bytes */
typedef byte rec_t;



/** Gets a bit field from within 1 byte. */
ulint rec_get_bit_field_1(
    const rec_t *rec, /*!< in: pointer to record origin */
    ulint offs,       /*!< in: offset from the origin down */
    ulint mask,       /*!< in: mask used to filter bits */
    ulint shift);      /*!< in: shift right applied after masking */


/** Gets a bit field from within 2 bytes. */
uint16_t rec_get_bit_field_2(
    const rec_t *rec, /*!< in: pointer to record origin */
    ulint offs,       /*!< in: offset from the origin down */
    ulint mask,       /*!< in: mask used to filter bits */
    ulint shift);      /*!< in: shift right applied after masking */


/** The following function retrieves the status bits of a new-style record.
 @return status bits */
MY_ATTRIBUTE((warn_unused_result)) ulint
    rec_get_status(const rec_t *rec); /*!< in: physical record */


#ifdef UNIV_DEBUG
/** Check if the info bits are valid.
@param[in]	bits	info bits to check
@return true if valid */
inline bool rec_info_bits_valid(ulint bits) {
  return (0 == (bits & ~(REC_INFO_DELETED_FLAG | REC_INFO_MIN_REC_FLAG |
                         REC_INFO_INSTANT_FLAG)));
}
#endif /* UNIV_DEBUG */

/** The following function is used to retrieve the info bits of a record.
@param[in]	rec	physical record
@param[in]	comp	nonzero=compact page format
@return info bits */
ulint rec_get_info_bits(const rec_t *rec, ulint comp);

/** The following function is used to retrieve the info bits of a temporary
record.
@param[in]	rec	physical record
@return	info bits */
ulint rec_get_info_bits_temp(const rec_t *rec);

/** The following function is used to get the number of fields
 in an old-style record, which is stored in the rec
 @return number of data fields */
MY_ATTRIBUTE((warn_unused_result)) uint16_t
    rec_get_n_fields_old_raw(const rec_t *rec); /*!< in: physical record */


/** The following function returns the number of allocated elements
 for an array of offsets.
 @return number of elements */
MY_ATTRIBUTE((warn_unused_result)) ulint rec_offs_get_n_alloc(
    const ulint *offsets); /*!< in: array for rec_get_offsets() */

/** The following function sets the number of allocated elements
 for an array of offsets. */
void rec_offs_set_n_alloc(ulint *offsets, /*!< out: array for rec_get_offsets(),
                                          must be allocated */
                          ulint n_alloc);  /*!< in: number of elements */


#define rec_offs_init(offsets) \
  rec_offs_set_n_alloc(offsets, (sizeof offsets) / sizeof *offsets)


/** The following function sets the number of fields in offsets. */
void rec_offs_set_n_fields(ulint *offsets, /*!< in/out: array returned by
                                           rec_get_offsets() */
                           ulint n_fields); /*!< in: number of fields */

/** The following function returns the number of fields in a record.
 @return number of fields */
MY_ATTRIBUTE((warn_unused_result)) ulint rec_offs_n_fields(
    const ulint *offsets); /*!< in: array returned by rec_get_offsets() */

/** The following function tells if a new-style record is instant record or not
@param[in]	rec	new-style record
@return true if it's instant affected */
bool rec_get_instant_flag_new(const rec_t *rec);

/** The following function tells if a new-style temporary record is instant
record or not
@param[in]	rec	new-style temporary record
@return	true if it's instant affected */
bool rec_get_instant_flag_new_temp(const rec_t *rec);

/** Get the number of fields for one new style leaf page record.
This is only needed for table after instant ADD COLUMN.
@param[in]	rec		leaf page record
@param[in]	extra_bytes	extra bytes of this record
@param[in,out]	length		length of number of fields
@return	number of fields */
uint32_t rec_get_n_fields_instant(const rec_t *rec, const ulint extra_bytes,
                                  uint16_t *length);

/** The following function is used to test whether the data offsets in the
 record are stored in one-byte or two-byte format.
 @return true if 1-byte form */
MY_ATTRIBUTE((warn_unused_result)) ibool
    rec_get_1byte_offs_flag(const rec_t *rec); /*!< in: physical record */

/** Returns the offset of nth field end if the record is stored in the 1-byte
 offsets form. If the field is SQL null, the flag is ORed in the returned
 value.
 @return offset of the start of the field, SQL null flag ORed */
MY_ATTRIBUTE((warn_unused_result)) ulint
    rec_1_get_field_end_info(const rec_t *rec, /*!< in: record */
                             ulint n);          /*!< in: field index */

/** Returns the offset of nth field end if the record is stored in the 2-byte
 offsets form. If the field is SQL null, the flag is ORed in the returned
 value.
 @return offset of the start of the field, SQL null flag and extern
 storage flag ORed */
MY_ATTRIBUTE((warn_unused_result)) ulint
    rec_2_get_field_end_info(const rec_t *rec, /*!< in: record */
                             ulint n);          /*!< in: field index */

/** The following function is used to get the pointer of the next chained record
 on the same page.
 @return pointer to the next chained record, or NULL if none */
const rec_t *rec_get_next_ptr_const(
    const rec_t *rec, /*!< in: physical record */
    ulint comp)       /*!< in: nonzero=compact page format */
    MY_ATTRIBUTE((warn_unused_result));

/** The following function is used to get the pointer of the next chained record
 on the same page.
 @return pointer to the next chained record, or NULL if none */
rec_t *rec_get_next_ptr(rec_t *rec, /*!< in: physical record */
                        ulint comp) /*!< in: nonzero=compact page format */
    MY_ATTRIBUTE((warn_unused_result));

/** The following function is used to get the offset of the
 next chained record on the same page.
 @return the page offset of the next chained record, or 0 if none */
ulint rec_get_next_offs(const rec_t *rec, /*!< in: physical record */
                        ulint comp) /*!< in: nonzero=compact page format */
    MY_ATTRIBUTE((warn_unused_result));

/** The following function is used to set the next record offset field of an
old-style record.
@param[in]	rec	old-style physical record
@param[in]	next	offset of the next record */
void rec_set_next_offs_old(rec_t *rec, ulint next);

/** The following function is used to set the next record offset field of a
new-style record. */
void rec_set_next_offs_new(rec_t *rec, ulint next);

/** The following function is used to get the number of records owned by the
 previous directory record.
 @return number of owned records */
ulint rec_get_n_owned_old(
    const rec_t *rec) /*!< in: old-style physical record */
    MY_ATTRIBUTE((warn_unused_result));

/** The following function is used to set the number of owned records.
@param[in]	rec		old-style physical record
@param[in]	n_owned		the number of owned */
void rec_set_n_owned_old(rec_t *rec, ulint n_owned);

/** Sets a bit field within 1 byte. */
void rec_set_bit_field_1(
    rec_t *rec,  /*!< in: pointer to record origin */
    ulint val,   /*!< in: value to set */
    ulint offs,  /*!< in: offset from the origin down */
    ulint mask,  /*!< in: mask used to filter bits */
    ulint shift); /*!< in: shift right applied after masking */

/** Sets a bit field within 2 bytes. */
void rec_set_bit_field_2(
    rec_t *rec,  /*!< in: pointer to record origin */
    ulint val,   /*!< in: value to set */
    ulint offs,  /*!< in: offset from the origin down */
    ulint mask,  /*!< in: mask used to filter bits */
    ulint shift); /*!< in: shift right applied after masking */


/** The following function is used to get the number of records owned by the
 previous directory record.
 @return number of owned records */
ulint rec_get_n_owned_new(
    const rec_t *rec) /*!< in: new-style physical record */
    MY_ATTRIBUTE((warn_unused_result));

/** The following function is used to set the info bits of a record.
@param[in]	rec	old-style physical record
@param[in]	bits	info bits */
void rec_set_info_bits_old(rec_t *rec, ulint bits);

/** The following function is used to set the info bits of a record.
@param[in,out]	rec	new-style physical record
@param[in]	bits	info bits */
void rec_set_info_bits_new(rec_t *rec, ulint bits);

/** The following function is used to set the status bits of a new-style record.
@param[in,out]	rec	physical record
@param[in]	bits	info bits */
void rec_set_status(rec_t *rec, ulint bits);

/** The following function is used to retrieve the info and status
 bits of a record.  (Only compact records have status bits.)
 @return info bits */
ulint rec_get_info_and_status_bits(
    const rec_t *rec, /*!< in: physical record */
    ulint comp)       /*!< in: nonzero=compact page format */
    MY_ATTRIBUTE((warn_unused_result));

/** The following function is used to set the info and status bits of a record.
(Only compact records have status bits.)
@param[in,out]	rec	compact physical record
@param[in]	bits	info bits */
void rec_set_info_and_status_bits(rec_t *rec, ulint bits);

/** The following function tells if record is delete marked.
 @return nonzero if delete marked */
ulint rec_get_deleted_flag(const rec_t *rec, /*!< in: physical record */
                           ulint comp) /*!< in: nonzero=compact page format */
    MY_ATTRIBUTE((warn_unused_result));

/** The following function is used to set the deleted bit.
@param[in]	rec		old-style physical record
@param[in]	flag		nonzero if delete marked */
void rec_set_deleted_flag_old(rec_t *rec, ulint flag);

/** The following function is used to set the instant bit.
@param[in,out]	rec	new-style physical record
@param[in]	flag	set the bit to this flag */
void rec_set_instant_flag_new(rec_t *rec, bool flag);


/** The following function tells if a new-style record is a node pointer.
 @return true if node pointer */
bool rec_get_node_ptr_flag(const rec_t *rec) /*!< in: physical record */
    MY_ATTRIBUTE((warn_unused_result));

/** The following function is used to get the order number of an old-style
record in the heap of the index page.
@param[in]	rec	physical record
@return heap order number */
ulint rec_get_heap_no_old(const rec_t *rec) MY_ATTRIBUTE((warn_unused_result));

/** The following function is used to set the heap number field in an old-style
record.
@param[in]	rec	physical record
@param[in]	heap_no	the heap number */
void rec_set_heap_no_old(rec_t *rec, ulint heap_no);

/** The following function is used to get the order number of a new-style
record in the heap of the index page.
@param[in]	rec	physical record
@return heap order number */
ulint rec_get_heap_no_new(const rec_t *rec) MY_ATTRIBUTE((warn_unused_result));

/** The following function is used to set the heap number field in a new-style
record.
@param[in,out]	rec	physical record
@param[in]	heap_no	the heap number */
void rec_set_heap_no_new(rec_t *rec, ulint heap_no);

/** The following function is used to set the 1-byte offsets flag.
@param[in]	rec	physical record
@param[in]	flag	TRUE if 1byte form */
void rec_set_1byte_offs_flag(rec_t *rec, ibool flag);

/** Returns nonzero if the field is stored off-page.
 @retval 0 if the field is stored in-page
 @retval REC_2BYTE_EXTERN_MASK if the field is stored externally */
ulint rec_2_is_field_extern(const rec_t *rec, /*!< in: record */
                            ulint n)          /*!< in: field index */
    MY_ATTRIBUTE((warn_unused_result));


/** The following function is used to get the offset to the nth
 data field in an old-style record.
 @return offset to the field */
ulint rec_get_nth_field_offs_old(
    const rec_t *rec, /*!< in: record */
    ulint n,          /*!< in: index of the field */
    ulint *len);      /*!< out: length of the field; UNIV_SQL_NULL
                      if SQL null */


#define rec_get_nth_field_old(rec, n, len) \
  ((rec) + rec_get_nth_field_offs_old(rec, n, len))


/** Returns the offset of nth field start if the record is stored in the 1-byte
 offsets form.
 @return offset of the start of the field */
ulint rec_1_get_field_start_offs(const rec_t *rec, /*!< in: record */
                                 ulint n);          /*!< in: field index */

/** Returns the offset of nth field start if the record is stored in the 2-byte
 offsets form.
 @return offset of the start of the field */
ulint rec_2_get_field_start_offs(const rec_t *rec, /*!< in: record */
                                 ulint n);          /*!< in: field index */

/** Returns the offset of n - 1th field end if the record is stored in the
 1-byte offsets form. If the field is SQL null, the flag is ORed in the returned
 value. This function and the 2-byte counterpart are defined here because the
 C-compiler was not able to sum negative and positive constant offsets, and
 warned of constant arithmetic overflow within the compiler.
 @return offset of the start of the PREVIOUS field, SQL null flag ORed */
ulint rec_1_get_prev_field_end_info(const rec_t *rec, /*!< in: record */
                                    ulint n);          /*!< in: field index */

/** Returns the offset of n - 1th field end if the record is stored in the
 2-byte offsets form. If the field is SQL null, the flag is ORed in the returned
 value.
 @return offset of the start of the PREVIOUS field, SQL null flag ORed */
ulint rec_2_get_prev_field_end_info(const rec_t *rec, /*!< in: record */
                                    ulint n);          /*!< in: field index */

/** Gets the physical size of an old-style field.
 Also an SQL null may have a field of size > 0,
 if the data type is of a fixed size.
 @return field size in bytes */
ulint rec_get_nth_field_size(const rec_t *rec, /*!< in: record */
                             ulint n)          /*!< in: index of the field */
    MY_ATTRIBUTE((warn_unused_result));

/** The following function is used to read the offset of the start of a data
 field in the record. The start of an SQL null field is the end offset of the
 previous non-null field, or 0, if none exists. If n is the number of the last
 field + 1, then the end offset of the last field is returned.
 @return offset of the start of the field */
ulint rec_get_field_start_offs(const rec_t *rec, /*!< in: record */
                               ulint n);          /*!< in: field index */

/** The following function is used to get an offset to the nth data field in a
record.
@param[in]	offsets	array returned by rec_get_offsets()
@param[in]	n	index of the field
@param[out]	len	length of the field; UNIV_SQL_NULL if SQL null;
                        UNIV_SQL_ADD_COL_DEFAULT if it's default value and no
value inlined
@return offset from the origin of rec */
ulint rec_get_nth_field_offs(const ulint *offsets, ulint n, ulint *len);


#ifdef UNIV_DEBUG
/** Gets the value of the specified field in the record.
This is used for normal cases, i.e. secondary index or clustered index
which must have no instantly added columns. Also note, if it's non-leaf
page records, it's OK to always use this functioni.
@param[in]	rec	physical record
@param[in]	offsets	array returned by rec_get_offsets()
@param[in]	n	index of the field
@param[out]	len	length of the field, UNIV_SQL_NULL if SQL null
@return value of the field */
inline byte *rec_get_nth_field(const rec_t *rec, const ulint *offsets, ulint n,
                               ulint *len) {
  ulint off = rec_get_nth_field_offs(offsets, n, len);
  ut_ad(*len != UNIV_SQL_ADD_COL_DEFAULT);
  return (const_cast<byte *>(rec) + off);
}
#else /* UNIV_DEBUG */
/** Gets the value of the specified field in the record.
This is used for normal cases, i.e. secondary index or clustered index
which must have no instantly added columns. Also note, if it's non-leaf
page records, it's OK to always use this functioni. */
#define rec_get_nth_field(rec, offsets, n, len) \
  ((rec) + rec_get_nth_field_offs(offsets, n, len))
#endif /* UNIV_DEBUG */


/** Determine if the field is not NULL and not having default value
after instant ADD COLUMN
@param[in]	len	length of a field
@return	true if not NULL and not having default value */
bool rec_field_not_null_not_add_col_def(ulint len);


/** Determine if the offsets are for a record in the new
 compact format.
 @return nonzero if compact format */
ulint rec_offs_comp(
    const ulint *offsets) /*!< in: array returned by rec_get_offsets() */
    MY_ATTRIBUTE((warn_unused_result));

/** Determine if the offsets are for a record containing
 externally stored columns.
 @return nonzero if externally stored */
ulint rec_offs_any_extern(
    const ulint *offsets) /*!< in: array returned by rec_get_offsets() */
    MY_ATTRIBUTE((warn_unused_result));

/** Returns nonzero if the SQL NULL bit is set in nth field of rec.
 @return nonzero if SQL NULL */
ulint rec_offs_nth_sql_null(
    const ulint *offsets, /*!< in: array returned by rec_get_offsets() */
    ulint n);              /*!< in: nth field */

/** Returns nonzero if the default bit is set in nth field of rec.
@return nonzero if default bit is set */
ulint rec_offs_nth_default(const ulint *offsets, ulint n);

/** Gets the physical size of a field.
 @return length of field */
ulint rec_offs_nth_size(
    const ulint *offsets, /*!< in: array returned by rec_get_offsets() */
    ulint n);              /*!< in: nth field */

/** Returns nonzero if the extern bit is set in nth field of rec.
 @return nonzero if externally stored */
ulint rec_offs_nth_extern(
    const ulint *offsets, /*!< in: array returned by rec_get_offsets() */
    ulint n);              /*!< in: nth field */

/** Returns the number of extern bits set in a record.
 @return number of externally stored fields */
ulint rec_offs_n_extern(
    const ulint *offsets); /*!< in: array returned by rec_get_offsets() */

/** Sets the field end info for the nth field if the record is stored in the
 1-byte format. */
void rec_1_set_field_end_info(rec_t *rec, /*!< in: record */
                              ulint n,    /*!< in: field index */
                              ulint info); /*!< in: value to set */


/** Sets the field end info for the nth field if the record is stored in the
 2-byte format. */
void rec_2_set_field_end_info(rec_t *rec, /*!< in: record */
                              ulint n,    /*!< in: field index */
                              ulint info); /*!< in: value to set */


/** This is used to modify the value of an already existing field in a record.
 The previous value must have exactly the same size as the new value. If len
 is UNIV_SQL_NULL then the field is treated as an SQL null.
 For records in ROW_FORMAT=COMPACT (new-style records), len must not be
 UNIV_SQL_NULL unless the field already is SQL null. */
void rec_set_nth_field(
    rec_t *rec,           /*!< in: record */
    const ulint *offsets, /*!< in: array returned by rec_get_offsets() */
    ulint n,              /*!< in: index number of the field */
    const void *data,     /*!< in: pointer to the data
                          if not SQL null */
    ulint len);            /*!< in: length of the data or UNIV_SQL_NULL */

/** Writes an SQL null field full of zeros. */
void data_write_sql_null(byte *data, /*!< in: pointer to a buffer of size len */
                         ulint len);  /*!< in: SQL null size in bytes */

/** Sets an old-style record field to SQL null.
 The physical size of the field is not changed. */
void rec_set_nth_field_sql_null(rec_t *rec, /*!< in: record */
                                ulint n);    /*!< in: index of the field */

/** Sets the value of the ith field SQL null bit of an old-style record. */
void rec_set_nth_field_null_bit(rec_t *rec, /*!< in: record */
                                ulint i,    /*!< in: ith field */
                                ibool val); /*!< in: value to set */

/** The following function is used to set the number of fields
 in an old-style record. */
void rec_set_n_fields_old(rec_t *rec,     /*!< in: physical record */
                          ulint n_fields); /*!< in: the number of fields */

/** The following function returns the data size of an old-style physical
 record, that is the sum of field lengths. SQL null fields
 are counted as length 0 fields. The value returned by the function
 is the distance from record origin to record end in bytes.
 @return size */
ulint rec_get_data_size_old(const rec_t *rec); /*!< in: physical record */

/** The following function returns the data size of a physical
 record, that is the sum of field lengths. SQL null fields
 are counted as length 0 fields. The value returned by the function
 is the distance from record origin to record end in bytes.
 @return size */
ulint rec_offs_data_size(
    const ulint *offsets); /*!< in: array returned by rec_get_offsets() */

/** Returns the total size of record minus data size of record. The value
 returned by the function is the distance from record start to record origin
 in bytes.
 @return size */
ulint rec_offs_extra_size(
    const ulint *offsets); /*!< in: array returned by rec_get_offsets() */

/** Returns the total size of a physical record.
 @return size */
ulint rec_offs_size(
    const ulint *offsets); /*!< in: array returned by rec_get_offsets() */


#ifdef UNIV_DEBUG
/** Returns a pointer to the end of the record.
 @return pointer to end */
UNIV_INLINE
byte *rec_get_end(
    const rec_t *rec,     /*!< in: pointer to record */
    const ulint *offsets) /*!< in: array returned by rec_get_offsets() */
{
  ut_ad(rec_offs_validate(rec, NULL, offsets));
  return (const_cast<rec_t *>(rec + rec_offs_data_size(offsets)));
}

/** Returns a pointer to the start of the record.
 @return pointer to start */
UNIV_INLINE
byte *rec_get_start(
    const rec_t *rec,     /*!< in: pointer to record */
    const ulint *offsets) /*!< in: array returned by rec_get_offsets() */
{
  ut_ad(rec_offs_validate(rec, NULL, offsets));
  return (const_cast<rec_t *>(rec - rec_offs_extra_size(offsets)));
}
#endif /* UNIV_DEBUG */


/** Copy a physical record to a buffer.
@param[in]	buf	buffer
@param[in]	rec	physical record
@param[in]	offsets	array returned by rec_get_offsets()
@return pointer to the origin of the copy */
rec_t *rec_copy(void *buf, const rec_t *rec, const ulint *offsets);



/** Returns the extra size of an old-style physical record if we know its
 data size and number of fields.
 @return extra size */
ulint rec_get_converted_extra_size(
    ulint data_size, /*!< in: data size */
    ulint n_fields,  /*!< in: number of fields */
    ulint n_ext);     /*!< in: number of externally stored columns */
