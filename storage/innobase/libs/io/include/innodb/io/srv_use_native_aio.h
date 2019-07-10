#pragma once

#include <innodb/univ/univ.h>

/* If this flag is TRUE, then we will use the native aio of the
OS (provided we compiled Innobase with it in), otherwise we will
use simulated aio we build below with threads.
Currently we support native aio on windows and linux */
extern bool srv_use_native_aio;
