#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_mem/dict_vcol_templ_t.h>
#include <innodb/allocator/ut_free.h>

/** Free the virtual column template
@param[in,out]	vc_templ	virtual column template */
UNIV_INLINE
void dict_free_vc_templ(dict_vcol_templ_t *vc_templ) {
  if (vc_templ->vtempl != NULL) {
    ut_ad(vc_templ->n_v_col > 0);
    for (ulint i = 0; i < vc_templ->n_col + vc_templ->n_v_col; i++) {
      if (vc_templ->vtempl[i] != NULL) {
        ut_free(vc_templ->vtempl[i]);
      }
    }
    ut_free(vc_templ->default_rec);
    ut_free(vc_templ->vtempl);
    vc_templ->vtempl = NULL;
  }
}
