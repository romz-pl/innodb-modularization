#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>

struct fil_node_t;
class IORequest;

dberr_t os_aio_linux_handler(ulint global_segment, fil_node_t **m1,
                                    void **m2, IORequest *request);
