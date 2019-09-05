#pragma once

#include <innodb/univ/univ.h>

struct sess_t;

/** Dummy session used currently in MySQL interface */
extern sess_t *trx_dummy_sess;
