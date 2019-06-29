#pragma once

#include <innodb/univ/univ.h>
#include <innodb/sync_latch/LatchMetaData.h>

#ifndef UNIV_LIBRARY

/** Note: This is accessed without any mutex protection. It is initialised
at startup and elements should not be added to or removed from it after
that.  See sync_latch_meta_init() */
extern LatchMetaData latch_meta;

#endif
