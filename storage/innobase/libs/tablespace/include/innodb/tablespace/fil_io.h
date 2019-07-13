#pragma once

#include <innodb/univ/univ.h>

#include <innodb/tablespace/space_id_t.h>
#include <innodb/tablespace/page_id_t.h>
#include <innodb/error/dberr_t.h>

class IORequest;
class page_size_t;

dberr_t fil_io(const IORequest &type, bool sync, const page_id_t &page_id,
               const page_size_t &page_size, ulint byte_offset, ulint len,
               void *buf, void *message) MY_ATTRIBUTE((warn_unused_result));
