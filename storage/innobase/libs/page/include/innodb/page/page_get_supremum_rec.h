#pragma once

#include <innodb/univ/univ.h>

#include <innodb/page/page_get_supremum_offset.h>

#define page_get_supremum_rec(page) ((page) + page_get_supremum_offset(page))
