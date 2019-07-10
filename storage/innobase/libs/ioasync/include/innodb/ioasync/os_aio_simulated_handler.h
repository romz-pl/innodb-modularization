#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>

class IORequest;
struct fil_node_t;

dberr_t os_aio_simulated_handler(ulint global_segment, fil_node_t **m1,
                                        void **m2, IORequest *type);
