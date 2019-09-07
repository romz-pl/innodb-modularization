#include <innodb/trx_trx/trx_resurrect_table_ids.h>

#include <innodb/mtr/mtr_commit.h>
#include <innodb/mtr/mtr_start.h>
#include <innodb/trx_trx/trx_state_eq.h>
#include <innodb/trx_trx/resurrected_trx_tables.h>
#include <innodb/disk/page_t.h>
#include <innodb/trx_types/trx_undo_rec_t.h>
#include <innodb/trx_undo/trx_undo_page_get.h>
#include <innodb/page/page_align.h>
#include <innodb/trx_rec/trx_undo_rec_get_pars.h>
#include <innodb/trx_undo/trx_undo_get_prev_rec.h>
#include <innodb/trx_undo/trx_undo_t.h>
#include <innodb/trx_rec/type_cmpl_t.h>

/** Resurrect the table IDs for a resurrected transaction.
@param[in]	trx		resurrected transaction
@param[in]	undo_ptr	pointer to undo segment
@param[in]	undo		undo log */
void trx_resurrect_table_ids(trx_t *trx, const trx_undo_ptr_t *undo_ptr,
                                    const trx_undo_t *undo) {
  mtr_t mtr;
  page_t *undo_page;
  trx_undo_rec_t *undo_rec;

  ut_ad(undo == undo_ptr->insert_undo || undo == undo_ptr->update_undo);

  if (trx_state_eq(trx, TRX_STATE_COMMITTED_IN_MEMORY) || undo->empty) {
    return;
  }

  table_id_set empty;
  table_id_set &tables =
      resurrected_trx_tables.insert(trx_table_map::value_type(trx, empty))
          .first->second;

  mtr_start(&mtr);

  /* trx_rseg_mem_create() may have acquired an X-latch on this
  page, so we cannot acquire an S-latch. */
  undo_page = trx_undo_page_get(page_id_t(undo->space, undo->top_page_no),
                                undo->page_size, &mtr);

  undo_rec = undo_page + undo->top_offset;

  do {
    ulint type;
    undo_no_t undo_no;
    table_id_t table_id;
    ulint cmpl_info;
    bool updated_extern;
    type_cmpl_t type_cmpl;

    page_t *undo_rec_page = page_align(undo_rec);

    if (undo_rec_page != undo_page) {
      mtr.release_page(undo_page, MTR_MEMO_PAGE_X_FIX);
      undo_page = undo_rec_page;
    }

    trx_undo_rec_get_pars(undo_rec, &type, &cmpl_info, &updated_extern,
                          &undo_no, &table_id, type_cmpl);
    tables.insert(table_id);

    undo_rec = trx_undo_get_prev_rec(undo_rec, undo->hdr_page_no,
                                     undo->hdr_offset, false, &mtr);
  } while (undo_rec);

  mtr_commit(&mtr);
}
