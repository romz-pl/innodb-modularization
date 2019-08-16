#include <innodb/record/rec_get_trx_id.h>

#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/memory/mem_heap_free.h>
#include <innodb/record/flag.h>
#include <innodb/record/rec_get_nth_field.h>
#include <innodb/record/rec_get_offsets.h>
#include <innodb/record/rec_offs_init.h>


trx_id_t trx_read_trx_id(const byte *ptr);

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
