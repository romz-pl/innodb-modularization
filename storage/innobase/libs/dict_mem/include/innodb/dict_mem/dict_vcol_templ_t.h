#pragma once

#include <innodb/univ/univ.h>

#include <string>

struct mysql_row_templ_t;

/** Structure defines template related to virtual columns and
their base columns */
struct dict_vcol_templ_t {
  /** number of regular columns */
  ulint n_col;

  /** number of virtual columns */
  ulint n_v_col;

  /** array of templates for virtual col and their base columns */
  mysql_row_templ_t **vtempl;

  /** table's database name */
  std::string db_name;

  /** table name */
  std::string tb_name;

  /** share->table_name */
  std::string share_name;

  /** MySQL record length */
  ulint rec_len;

  /** default column value if any */
  byte *default_rec;
};
