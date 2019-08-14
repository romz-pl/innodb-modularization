#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_col_t.h>
#include <innodb/dict_mem/dict_field_t.h>
#include <innodb/dict_mem/flags.h>
#include <innodb/dict_mem/rec_cache_t.h>
#include <innodb/dict_mem/zip_pad_info_t.h>
#include <innodb/dict_types/id_name_t.h>
#include <innodb/gis_type/rtr_info_track_t.h>
#include <innodb/gis_type/rtr_ssn_t.h>
#include <innodb/memory/mem_heap_t.h>
#include <innodb/sync_rw/rw_lock_t.h>

#include "include/mysql/plugin_ftparser.h"
#include "sql/sql_const.h" /* MAX_KEY_LENGTH */

struct dict_table_t;
struct dict_field_t;
class last_ops_cur_t;
struct btr_search_t;
struct row_log_t;
struct trx_t;


namespace dd { class Spatial_reference_system; }

/** Data structure for an index.  Most fields will be
initialized to 0, NULL or FALSE in dict_mem_index_create(). */
struct dict_index_t {
  space_index_t id;       /*!< id of the index */
  mem_heap_t *heap;       /*!< memory heap */
  id_name_t name;         /*!< index name */
  const char *table_name; /*!< table name */
  dict_table_t *table;    /*!< back pointer to table */
  unsigned space : 32;
  /*!< space where the index tree is placed */
  unsigned page : 32; /*!< index tree root page number */
  unsigned merge_threshold : 6;
  /*!< In the pessimistic delete, if the page
  data size drops below this limit in percent,
  merging it to a neighbor is tried */
#define DICT_INDEX_MERGE_THRESHOLD_DEFAULT 50
  unsigned type : DICT_IT_BITS;
  /*!< index type (DICT_CLUSTERED, DICT_UNIQUE,
  DICT_IBUF, DICT_CORRUPT) */
#define MAX_KEY_LENGTH_BITS 12
  unsigned trx_id_offset : MAX_KEY_LENGTH_BITS;
  /*!< position of the trx id column
  in a clustered index record, if the fields
  before it are known to be of a fixed size,
  0 otherwise */
#if (1 << MAX_KEY_LENGTH_BITS) < MAX_KEY_LENGTH
#error(1<<MAX_KEY_LENGTH_BITS) < MAX_KEY_LENGTH
#endif
  unsigned n_user_defined_cols : 10;
  /*!< number of columns the user defined to
  be in the index: in the internal
  representation we add more columns */
  unsigned allow_duplicates : 1;
  /*!< if true, allow duplicate values
  even if index is created with unique
  constraint */
  unsigned nulls_equal : 1;
  /*!< if true, SQL NULL == SQL NULL */
  unsigned disable_ahi : 1;
  /*!< if true, then disable AHI. Currently
  limited to intrinsic temporary table and SDI
  table as index id is not unique for such table
  which is one of the validation criterion for
  ahi. */
  unsigned n_uniq : 10;     /*!< number of fields from the beginning
                          which are enough to determine an index
                          entry uniquely */
  unsigned n_def : 10;      /*!< number of fields defined so far */
  unsigned n_fields : 10;   /*!< number of fields in the index */
  unsigned n_nullable : 10; /*!< number of nullable fields */
  unsigned n_instant_nullable : 10;
  /*!< number of nullable fields before first
  instant ADD COLUMN applied to this table.
  This is valid only when has_instant_cols() is true */
  unsigned cached : 1; /*!< TRUE if the index object is in the
                      dictionary cache */
  unsigned to_be_dropped : 1;
  /*!< TRUE if the index is to be dropped;
  protected by dict_operation_lock */
  unsigned online_status : 2;
  /*!< enum online_index_status.
  Transitions from ONLINE_INDEX_COMPLETE (to
  ONLINE_INDEX_CREATION) are protected
  by dict_operation_lock and
  dict_sys->mutex. Other changes are
  protected by index->lock. */
  unsigned uncommitted : 1;
  /*!< a flag that is set for secondary indexes
  that have not been committed to the
  data dictionary yet */
  unsigned instant_cols : 1;
  /*!< TRUE if the index is clustered index and it has some
  instant columns */
  uint32_t srid; /* spatial reference id */
  bool srid_is_valid;
  /* says whether SRID is valid - it cane be
  undefined */
  std::unique_ptr<dd::Spatial_reference_system> rtr_srs;
  /*!< Cached spatial reference system dictionary
  entry used by R-tree indexes. */

#ifdef UNIV_DEBUG
  uint32_t magic_n; /*!< magic number */
/** Value of dict_index_t::magic_n */
#define DICT_INDEX_MAGIC_N 76789786
#endif
  dict_field_t *fields; /*!< array of field descriptions */
#ifndef UNIV_HOTBACKUP
  st_mysql_ftparser *parser; /*!< fulltext parser plugin */
  bool is_ngram;
  /*!< true if it's ngram parser */
  bool has_new_v_col;
  /*!< whether it has a newly added virtual
  column in ALTER */
  bool hidden; /*!< if the index is an hidden index */
#endif         /* !UNIV_HOTBACKUP */
  UT_LIST_NODE_T(dict_index_t)
  indexes; /*!< list of indexes of the table */
  btr_search_t *search_info;
  /*!< info used in optimistic searches */
#ifndef UNIV_HOTBACKUP
  row_log_t *online_log;
  /*!< the log of modifications
  during online index creation;
  valid when online_status is
  ONLINE_INDEX_CREATION */
  /*----------------------*/
  /** Statistics for query optimization */
  /* @{ */
  ib_uint64_t *stat_n_diff_key_vals;
  /*!< approximate number of different
  key values for this index, for each
  n-column prefix where 1 <= n <=
  dict_get_n_unique(index) (the array is
  indexed from 0 to n_uniq-1); we
  periodically calculate new
  estimates */
  ib_uint64_t *stat_n_sample_sizes;
  /*!< number of pages that were sampled
  to calculate each of stat_n_diff_key_vals[],
  e.g. stat_n_sample_sizes[3] pages were sampled
  to get the number stat_n_diff_key_vals[3]. */
  ib_uint64_t *stat_n_non_null_key_vals;
  /* approximate number of non-null key values
  for this index, for each column where
  1 <= n <= dict_get_n_unique(index) (the array
  is indexed from 0 to n_uniq-1); This
  is used when innodb_stats_method is
  "nulls_ignored". */
  ulint stat_index_size;
  /*!< approximate index size in
  database pages */
#endif /* !UNIV_HOTBACKUP */
  ulint stat_n_leaf_pages;
  /*!< approximate number of leaf pages in the
  index tree */
  /* @} */
  last_ops_cur_t *last_ins_cur;
  /*!< cache the last insert position.
  Currently limited to auto-generated
  clustered index on intrinsic table only. */
  last_ops_cur_t *last_sel_cur;
  /*!< cache the last selected position
  Currently limited to intrinsic table only. */
  rec_cache_t rec_cache;
  /*!< cache the field that needs to be
  re-computed on each insert.
  Limited to intrinsic table as this is common
  share and can't be used without protection
  if table is accessible to multiple-threads. */
  rtr_ssn_t rtr_ssn;           /*!< Node sequence number for RTree */
  rtr_info_track_t *rtr_track; /*!< tracking all R-Tree search cursors */
  trx_id_t trx_id;             /*!< id of the transaction that created this
                               index, or 0 if the index existed
                               when InnoDB was started up */
  zip_pad_info_t zip_pad;      /*!< Information about state of
                               compression failures and successes */
  rw_lock_t lock;              /*!< read-write lock protecting the
                               upper levels of the index tree */
  bool fill_dd;                /*!< Flag whether need to fill dd tables
                               when it's a fulltext index. */

  /** Determine if the index has been committed to the
  data dictionary.
  @return whether the index definition has been committed */
  bool is_committed() const {
    ut_ad(!uncommitted || !(type & DICT_CLUSTERED));
    return (UNIV_LIKELY(!uncommitted));
  }

  /** Flag an index committed or uncommitted.
  @param[in]	committed	whether the index is committed */
  void set_committed(bool committed) {
    ut_ad(!to_be_dropped);
    ut_ad(committed || !(type & DICT_CLUSTERED));
    uncommitted = !committed;
  }

  /** Get the next index.
  @return	next index
  @retval	NULL	if this was the last index */
  const dict_index_t *next() const {
    const dict_index_t *next = UT_LIST_GET_NEXT(indexes, this);
    ut_ad(magic_n == DICT_INDEX_MAGIC_N);
    return (next);
  }
  /** Get the next index.
  @return	next index
  @retval	NULL	if this was the last index */
  dict_index_t *next() {
    return (const_cast<dict_index_t *>(
        const_cast<const dict_index_t *>(this)->next()));
  }

  /** Check whether the index is corrupted.
  @return true if index is corrupted, otherwise false */
  bool is_corrupted() const {
    ut_ad(magic_n == DICT_INDEX_MAGIC_N);

    return (type & DICT_CORRUPT);
  }

  /* Check whether the index is the clustered index
  @return nonzero for clustered index, zero for other indexes */

  bool is_clustered() const {
    ut_ad(magic_n == DICT_INDEX_MAGIC_N);

    return (type & DICT_CLUSTERED);
  }

  /** Returns the minimum data size of an index record.
  @return minimum data size in bytes */
  ulint get_min_size() const {
    ulint size = 0;

    for (unsigned i = 0; i < n_fields; i++) {
      size += get_col(i)->get_min_size();
    }

    return (size);
  }

  /** Check whether index can be used by transaction
  @param[in] trx		transaction*/
  bool is_usable(const trx_t *trx) const;

  /** Check whether index has any instantly added columns
  @return true if this is instant affected, otherwise false */
  bool has_instant_cols() const { return (instant_cols); }

  /** Returns the number of nullable fields before specified
  nth field
  @param[in]	nth	nth field to check */
  uint32_t get_n_nullable_before(uint32_t nth) const {
    uint32_t nullable = n_nullable;

    ut_ad(nth <= n_fields);

    for (uint32_t i = nth; i < n_fields; ++i) {
      if (get_field(i)->col->is_nullable()) {
        --nullable;
      }
    }

    return (nullable);
  }

  /** Returns the number of fields before first instant ADD COLUMN */
  uint32_t get_instant_fields() const;

  /** Adds a field definition to an index. NOTE: does not take a copy
  of the column name if the field is a column. The memory occupied
  by the column name may be released only after publishing the index.
  @param[in] name_arg	column name
  @param[in] prefix_len	0 or the column prefix length in a MySQL index
                          like INDEX (textcol(25))
  @param[in] is_ascending	true=ASC, false=DESC */
  void add_field(const char *name_arg, ulint prefix_len, bool is_ascending) {
    dict_field_t *field;

    ut_ad(magic_n == DICT_INDEX_MAGIC_N);

    n_def++;

    field = get_field(n_def - 1);

    field->name = name_arg;
    field->prefix_len = (unsigned int)prefix_len;
    field->is_ascending = is_ascending;
  }

  /** Gets the nth field of an index.
  @param[in] pos	position of field
  @return pointer to field object */
  dict_field_t *get_field(ulint pos) const {
    ut_ad(pos < n_def);
    ut_ad(magic_n == DICT_INDEX_MAGIC_N);

    return (fields + pos);
  }

  /** Gets pointer to the nth column in an index.
  @param[in] pos	position of the field
  @return column */
  const dict_col_t *get_col(ulint pos) const { return (get_field(pos)->col); }

  /** Gets the column number the nth field in an index.
  @param[in] pos	position of the field
  @return column number */
  ulint get_col_no(ulint pos) const;

  /** Returns the position of a system column in an index.
  @param[in] type		DATA_ROW_ID, ...
  @return position, ULINT_UNDEFINED if not contained */
  ulint get_sys_col_pos(ulint type) const;

  /** Looks for column n in an index.
  @param[in]	n		column number
  @param[in]	inc_prefix	true=consider column prefixes too
  @param[in]	is_virtual	true==virtual column
  @return position in internal representation of the index;
  ULINT_UNDEFINED if not contained */
  ulint get_col_pos(ulint n, bool inc_prefix = false,
                    bool is_virtual = false) const;

  /** Get the default value of nth field and its length if exists.
  If not exists, both the return value is nullptr and length is 0.
  @param[in]	nth	nth field to get
  @param[in,out]	length	length of the default value
  @return	the default value data of nth field */
  const byte *get_nth_default(ulint nth, ulint *length) const {
    ut_ad(nth < n_fields);
    ut_ad(get_instant_fields() <= nth);
    const dict_col_t *col = get_col(nth);
    if (col->instant_default == nullptr) {
      *length = 0;
      return (nullptr);
    }

    *length = col->instant_default->len;
    ut_ad(*length == 0 || *length == UNIV_SQL_NULL ||
          col->instant_default->value != nullptr);
    return (col->instant_default->value);
  }

  /** Sets srid and srid_is_valid values
  @param[in]	srid_value		value of SRID, may be garbage
                                          if srid_is_valid_value = false
  @param[in]	srid_is_valid_value	value of srid_is_valid */
  void fill_srid_value(uint32_t srid_value, bool srid_is_valid_value) {
    srid_is_valid = srid_is_valid_value;
    srid = srid_value;
  }

  /** Check if the underlying table is compressed.
  @return true if compressed, false otherwise. */
  bool is_compressed() const;
};
