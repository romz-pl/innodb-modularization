#pragma once

#include <innodb/univ/univ.h>

struct page_zip_des_t;

ibool page_zip_simple_validate(
    const page_zip_des_t *page_zip);
