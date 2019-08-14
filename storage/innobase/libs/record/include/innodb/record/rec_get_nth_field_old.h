#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/rec_t.h>
#include <innodb/record/rec_get_nth_field_offs_old.h>

#define rec_get_nth_field_old(rec, n, len) \
  ((rec) + rec_get_nth_field_offs_old(rec, n, len))
