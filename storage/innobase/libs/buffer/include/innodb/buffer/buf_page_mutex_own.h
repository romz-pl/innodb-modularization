#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

/** Test if block->mutex is owned. */
#define buf_page_mutex_own(b) (b)->mutex.is_owned()

#endif
