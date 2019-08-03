#pragma once

#include <innodb/univ/univ.h>

#include <limits>

/** Page number */
typedef uint32_t page_no_t;

/** 'null' (undefined) page offset in the context of file spaces */
constexpr page_no_t FIL_NULL = std::numeric_limits<page_no_t>::max();

/** Maximum Page Number, one less than FIL_NULL */
constexpr page_no_t PAGE_NO_MAX = std::numeric_limits<page_no_t>::max() - 1;

