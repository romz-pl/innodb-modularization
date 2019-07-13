#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>

class page_id_t;
class page_size_t;

dberr_t fil_write(const page_id_t &page_id, const page_size_t &page_size,
                         ulint byte_offset, ulint len, void *buf);
