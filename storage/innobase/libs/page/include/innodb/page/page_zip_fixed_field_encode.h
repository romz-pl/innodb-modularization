#pragma once

#include <innodb/univ/univ.h>

byte *page_zip_fixed_field_encode(
    byte *buf, /*!< in: pointer to buffer where to write */
    ulint val); /*!< in: value to write */
