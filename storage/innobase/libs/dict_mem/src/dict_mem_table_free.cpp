#include <innodb/dict_mem/dict_mem_table_free.h>

#include <innodb/allocator/UT_DELETE.h>
#include <innodb/allocator/ut_free.h>
#include <innodb/dict_mem/dict_table_autoinc_destroy.h>
#include <innodb/dict_mem/dict_table_get_nth_v_col.h>
#include <innodb/dict_mem/dict_table_has_fts_index.h>
#include <innodb/dict_mem/dict_table_mutex_destroy.h>
#include <innodb/dict_mem/dict_table_stats_latch_destroy.h>
#include <innodb/dict_mem/dict_table_t.h>
#include <innodb/dict_mem/dict_v_col_t.h>
#include <innodb/memory/mem_heap_free.h>


void fts_optimize_remove_table(dict_table_t *table);
void fts_free(dict_table_t *table);

/** Free a table memory object. */
void dict_mem_table_free(dict_table_t *table) /*!< in: table */
{
  ut_ad(table);
  ut_ad(table->magic_n == DICT_TABLE_MAGIC_N);
  ut_d(table->cached = FALSE);

#ifndef UNIV_HOTBACKUP
#ifndef UNIV_LIBRARY
  if (dict_table_has_fts_index(table) ||
      DICT_TF2_FLAG_IS_SET(table, DICT_TF2_FTS_HAS_DOC_ID) ||
      DICT_TF2_FLAG_IS_SET(table, DICT_TF2_FTS_ADD_DOC_ID)) {
    if (table->fts) {
      fts_optimize_remove_table(table);

      fts_free(table);
    }
  }

  dict_table_mutex_destroy(table);

  dict_table_autoinc_destroy(table);

  dict_table_stats_latch_destroy(table);

  table->foreign_set.~dict_foreign_set();
  table->referenced_set.~dict_foreign_set();
#endif /* !UNIV_LIBRARY */
#endif /* !UNIV_HOTBACKUP */

  ut_free(table->name.m_name);
  table->name.m_name = NULL;

#ifndef UNIV_HOTBACKUP
#ifndef UNIV_LIBRARY
  /* Clean up virtual index info structures that are registered
  with virtual columns */
  for (ulint i = 0; i < table->n_v_def; i++) {
    dict_v_col_t *vcol = dict_table_get_nth_v_col(table, i);

    UT_DELETE(vcol->v_indexes);
  }
#endif /* !UNIV_LIBRARY */
#endif /* !UNIV_HOTBACKUP */

  if (table->s_cols != NULL) {
    UT_DELETE(table->s_cols);
  }

#ifndef UNIV_HOTBACKUP
  if (table->temp_prebuilt != NULL) {
    ut_ad(table->is_intrinsic());
    UT_DELETE(table->temp_prebuilt);
  }
#endif /* !UNIV_HOTBACKUP */

  mem_heap_free(table->heap);
}
