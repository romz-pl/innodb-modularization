#pragma once

#include <innodb/univ/univ.h>

#include <innodb/allocator/ut_allocator.h>

#include <set>

struct trx_t;

/** Set of table_id */
typedef std::set<table_id_t, std::less<table_id_t>, ut_allocator<table_id_t>>
    table_id_set;
