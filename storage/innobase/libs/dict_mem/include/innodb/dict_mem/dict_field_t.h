#pragma once

#include <innodb/univ/univ.h>

#include <innodb/dict_types/id_name_t.h>

struct dict_col_t;

/** Data structure for a field in an index */
struct dict_field_t {
  dict_field_t() : col(nullptr), prefix_len(0), fixed_len(0), is_ascending(0) {}

  dict_col_t *col;           /*!< pointer to the table column */
  id_name_t name;            /*!< name of the column */
  unsigned prefix_len : 12;  /*!< 0 or the length of the column
                             prefix in bytes in a MySQL index of
                             type, e.g., INDEX (textcol(25));
                             must be smaller than
                             DICT_MAX_FIELD_LEN_BY_FORMAT;
                             NOTE that in the UTF-8 charset, MySQL
                             sets this to (mbmaxlen * the prefix len)
                             in UTF-8 chars */
  unsigned fixed_len : 10;   /*!< 0 or the fixed length of the
                             column if smaller than
                             DICT_ANTELOPE_MAX_INDEX_COL_LEN */
  unsigned is_ascending : 1; /*!< 0=DESC, 1=ASC */
};
