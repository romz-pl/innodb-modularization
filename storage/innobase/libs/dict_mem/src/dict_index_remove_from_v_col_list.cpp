#include <innodb/dict_mem/dict_index_remove_from_v_col_list.h>

#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/dict_mem/dict_index_has_virtual.h>
#include <innodb/dict_mem/dict_index_get_n_fields.h>
#include <innodb/dict_mem/dict_v_idx_list.h>
#include <innodb/dict_mem/dict_v_col_t.h>

/** Clears the virtual column's index list before index is being freed.
@param[in]  index   Index being freed */
void dict_index_remove_from_v_col_list(dict_index_t *index) {
  /* Index is not completely formed */
  if (!index->cached) {
    return;
  }
  if (dict_index_has_virtual(index)) {
    const dict_col_t *col;
    const dict_v_col_t *vcol;

    for (ulint i = 0; i < dict_index_get_n_fields(index); i++) {
      col = index->get_col(i);
      if (col->is_virtual()) {
        vcol = reinterpret_cast<const dict_v_col_t *>(col);
        /* This could be NULL, when we do add
        virtual column, add index together. We do not
        need to track this virtual column's index */
        if (vcol->v_indexes == NULL) {
          continue;
        }
        dict_v_idx_list::iterator it;
        for (it = vcol->v_indexes->begin(); it != vcol->v_indexes->end();
             ++it) {
          dict_v_idx_t v_index = *it;
          if (v_index.index == index) {
            vcol->v_indexes->erase(it);
            break;
          }
        }
      }
    }
  }
}
