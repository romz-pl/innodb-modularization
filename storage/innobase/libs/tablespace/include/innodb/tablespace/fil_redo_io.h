#pragma once

#include <innodb/univ/univ.h>

#include <innodb/page/page_size_t.h>
#include <innodb/error/dberr_t.h>

class page_id_t;
class IORequest;

dberr_t fil_redo_io(const IORequest &type, const page_id_t &page_id,
                    const page_size_t &page_size, ulint byte_offset, ulint len,
                    void *buf) MY_ATTRIBUTE((warn_unused_result));
