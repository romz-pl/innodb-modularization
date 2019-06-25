#pragma once

#include <innodb/univ/univ.h>

#include <thread>

/** Operating system thread native handle */
using os_thread_id_t = std::thread::native_handle_type;
