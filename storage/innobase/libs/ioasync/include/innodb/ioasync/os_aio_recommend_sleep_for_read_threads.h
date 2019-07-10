#pragma once

#include <innodb/univ/univ.h>

/** If the following is true, read i/o handler threads try to
wait until a batch of new read requests have been posted */
extern bool os_aio_recommend_sleep_for_read_threads;
