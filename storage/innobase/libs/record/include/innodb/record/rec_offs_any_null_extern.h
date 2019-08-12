#pragma once

#include <innodb/univ/univ.h>

#include <innodb/record/rec_t.h>

struct page_zip_des_t;

const byte *rec_offs_any_null_extern(
    const rec_t *rec,     /*!< in: record */
    const ulint *offsets);
