#pragma once

#include <innodb/univ/univ.h>
#include <innodb/sync_latch/LatchCounter.h>
#include <innodb/sync_latch/LatchMeta.h>

#ifndef UNIV_LIBRARY

    typedef LatchMeta<LatchCounter> latch_meta_t;

#endif
