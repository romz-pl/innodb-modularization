#include <innodb/dict_mem/dict_mem_fill_column_struct.h>

#include <innodb/dict_mem/dict_col_t.h>
#include <innodb/data_types/dtype_get_mblen.h>

/** This function populates a dict_col_t memory structure with
 supplied information. */
void dict_mem_fill_column_struct(
    dict_col_t *column, /*!< out: column struct to be
                        filled */
    ulint col_pos,      /*!< in: column position */
    ulint mtype,        /*!< in: main data type */
    ulint prtype,       /*!< in: precise type */
    ulint col_len)      /*!< in: column length */
{
  column->ind = (unsigned int)col_pos;
  column->ord_part = 0;
  column->max_prefix = 0;
  column->mtype = (unsigned int)mtype;
  column->prtype = (unsigned int)prtype;
  column->len = (unsigned int)col_len;
  column->instant_default = nullptr;
#ifndef UNIV_HOTBACKUP
#ifndef UNIV_LIBRARY
  ulint mbminlen;
  ulint mbmaxlen;
  dtype_get_mblen(mtype, prtype, &mbminlen, &mbmaxlen);
  column->set_mbminmaxlen(mbminlen, mbmaxlen);
#endif /* !UNIV_LIBRARY */
#endif /* !UNIV_HOTBACKUP */
}
