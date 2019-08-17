#include <innodb/page/page_zip_compress_node_ptrs.h>

#include <innodb/record/rec_get_offsets.h>
#include <innodb/record/rec_offs_data_size.h>
#include <innodb/record/rec_offs_extra_size.h>
#include <innodb/dict_mem/flags.h>
#include <innodb/memory_check/memory_check.h>
#include <innodb/record/flag.h>
#include <innodb/record/rec_get_heap_no_new.h>


/** Compress the records of a node pointer page.
 @return Z_OK, or a zlib error code */
int page_zip_compress_node_ptrs(
    FILE_LOGFILE z_stream *c_stream, /*!< in/out: compressed page stream */
    const rec_t **recs,              /*!< in: dense page directory
                                     sorted by address */
    ulint n_dense,                   /*!< in: size of recs[] */
    dict_index_t *index,             /*!< in: the index of the page */
    byte *storage,                   /*!< in: end of dense page directory */
    mem_heap_t *heap)                /*!< in: temporary memory heap */
{
  int err = Z_OK;
  ulint *offsets = NULL;

  do {
    const rec_t *rec = *recs++;

    offsets = rec_get_offsets(rec, index, offsets, ULINT_UNDEFINED, &heap);
    /* Only leaf nodes may contain externally stored columns. */
    ut_ad(!rec_offs_any_extern(offsets));

    UNIV_MEM_ASSERT_RW(rec, rec_offs_data_size(offsets));
    UNIV_MEM_ASSERT_RW(rec - rec_offs_extra_size(offsets),
                       rec_offs_extra_size(offsets));

    /* Compress the extra bytes. */
    c_stream->avail_in =
        static_cast<uInt>(rec - REC_N_NEW_EXTRA_BYTES - c_stream->next_in);

    if (c_stream->avail_in) {
      err = deflate(c_stream, Z_NO_FLUSH);
      if (UNIV_UNLIKELY(err != Z_OK)) {
        break;
      }
    }
    ut_ad(!c_stream->avail_in);

    /* Compress the data bytes, except node_ptr. */
    c_stream->next_in = (byte *)rec;
    c_stream->avail_in =
        static_cast<uInt>(rec_offs_data_size(offsets) - REC_NODE_PTR_SIZE);

    if (c_stream->avail_in) {
      err = deflate(c_stream, Z_NO_FLUSH);
      if (UNIV_UNLIKELY(err != Z_OK)) {
        break;
      }
    }

    ut_ad(!c_stream->avail_in);

    memcpy(storage - REC_NODE_PTR_SIZE * (rec_get_heap_no_new(rec) - 1),
           c_stream->next_in, REC_NODE_PTR_SIZE);
    c_stream->next_in += REC_NODE_PTR_SIZE;
  } while (--n_dense);

  return (err);
}
