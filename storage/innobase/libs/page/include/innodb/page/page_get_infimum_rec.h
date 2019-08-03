#pragma once

#include <innodb/univ/univ.h>

#include <innodb/page/page_get_infimum_offset.h>

#define page_get_infimum_rec(page) ((page) + page_get_infimum_offset(page))
