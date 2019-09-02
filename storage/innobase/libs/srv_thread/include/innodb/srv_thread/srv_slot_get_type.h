#pragma once

#include <innodb/univ/univ.h>

#include <innodb/srv_thread/srv_thread_type.h>

struct srv_slot_t;

srv_thread_type srv_slot_get_type(
    const srv_slot_t *slot);
