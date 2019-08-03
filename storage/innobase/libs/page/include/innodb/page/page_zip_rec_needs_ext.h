#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

class page_size_t;

ibool page_zip_rec_needs_ext(ulint rec_size, ulint comp, ulint n_fields,
                             const page_size_t &page_size);

#endif
