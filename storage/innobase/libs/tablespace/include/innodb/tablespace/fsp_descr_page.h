#pragma once

#include <innodb/univ/univ.h>

class page_id_t;
class page_size_t;

ibool fsp_descr_page(const page_id_t &page_id, const page_size_t &page_size);
