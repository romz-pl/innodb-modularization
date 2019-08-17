#pragma once

#include <innodb/univ/univ.h>

struct dict_index_t;

dict_index_t *page_zip_fields_decode(const byte *buf, const byte *end,
                                            ulint *trx_id_col,
                                            bool is_spatial);
