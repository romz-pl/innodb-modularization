#pragma once

#include <innodb/univ/univ.h>

#include <innodb/univ/page_no_t.h>

bool recv_report_corrupt_log(const byte *ptr, int type, space_id_t space,
                                    page_no_t page_no);
