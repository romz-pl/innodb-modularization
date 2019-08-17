#pragma once

#include <innodb/univ/univ.h>


struct dict_index_t;

void page_zip_fields_free(dict_index_t *index);
