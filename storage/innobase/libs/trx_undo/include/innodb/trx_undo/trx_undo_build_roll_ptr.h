#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/page_no_t.h>

roll_ptr_t trx_undo_build_roll_ptr(ibool is_insert, space_id_t space_id,
                                          page_no_t page_no, ulint offset);
