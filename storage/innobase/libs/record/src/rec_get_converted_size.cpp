#include <innodb/record/rec_get_converted_size.h>

#include <innodb/data_types/dtuple_check_typed.h>
#include <innodb/data_types/dtuple_get_data_size.h>
#include <innodb/data_types/dtuple_get_info_bits.h>
#include <innodb/data_types/dtuple_get_n_fields.h>
#include <innodb/data_types/dtuple_t.h>
#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_mem/dict_table_is_comp.h>
#include <innodb/record/flag.h>
#include <innodb/record/rec_get_converted_extra_size.h>
#include <innodb/record/rec_get_converted_size_comp.h>


/** The following function returns the size of a data tuple when converted to
 a physical record.
 @return size */
ulint rec_get_converted_size(
    const dict_index_t *index, /*!< in: record descriptor */
    const dtuple_t *dtuple,    /*!< in: data tuple */
    ulint n_ext)               /*!< in: number of externally stored columns */
{
  ulint data_size;
  ulint extra_size;

  ut_ad(index);
  ut_ad(dtuple);
  ut_ad(dtuple_check_typed(dtuple));

  ut_ad(dict_index_is_ibuf(index)

        /* If this is an instant index and the tuple comes from
        UPDATE, its fields could be less than index definition */
        || index->has_instant_cols()

        || dtuple_get_n_fields(dtuple) ==
               (((dtuple_get_info_bits(dtuple) & REC_NEW_STATUS_MASK) ==
                 REC_STATUS_NODE_PTR)
                    ? dict_index_get_n_unique_in_tree_nonleaf(index) + 1
                    : dict_index_get_n_fields(index))

        /* a record for older SYS_INDEXES table
        (missing merge_threshold column) is acceptable. */
        || (index->table->id == DICT_INDEXES_ID &&
            dtuple_get_n_fields(dtuple) == dict_index_get_n_fields(index) - 1));

  if (dict_table_is_comp(index->table)) {
    return (rec_get_converted_size_comp(
        index, dtuple_get_info_bits(dtuple) & REC_NEW_STATUS_MASK,
        dtuple->fields, dtuple->n_fields, NULL));
  }

  data_size = dtuple_get_data_size(dtuple, 0);

  extra_size = rec_get_converted_extra_size(data_size,
                                            dtuple_get_n_fields(dtuple), n_ext);

#if 0
    /* This code is inactive since it may be the wrong place to add
    in the size of node pointers used in parent pages AND it is not
    currently needed since ha_innobase::max_supported_key_length()
    ensures that the key size limit for each page size is well below
    the actual limit ((free space on page / 4) - record overhead).
    But those limits will need to be raised when InnoDB can
    support multiple page sizes.  At that time, we will need
    to consider the node pointer on these universal btrees. */

    if (dict_index_is_ibuf(index)) {
        /* This is for the insert buffer B-tree.
        All fields in the leaf tuple ascend to the
        parent node plus the child page pointer. */

        /* ibuf cannot contain externally stored fields */
        ut_ad(n_ext == 0);

        /* Add the data pointer and recompute extra_size
        based on one more field. */
        data_size += REC_NODE_PTR_SIZE;
        extra_size = rec_get_converted_extra_size(
            data_size,
            dtuple_get_n_fields(dtuple) + 1,
            0);

        /* Be sure dtuple->n_fields has this node ptr
        accounted for.  This function should correspond to
        what rec_convert_dtuple_to_rec() needs in storage.
        In optimistic insert or update-not-in-place, we will
        have to ensure that if the record is converted to a
        node pointer, it will not become too large.*/
    }
#endif

  return (data_size + extra_size);
}



