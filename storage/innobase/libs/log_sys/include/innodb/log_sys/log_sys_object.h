#pragma once

#include <innodb/univ/univ.h>

#include <innodb/allocator/aligned_pointer.h>
#include <innodb/log_types/log_t.h>

/** Redo log system. Singleton used to populate global pointer. */
extern aligned_pointer<log_t> *log_sys_object;
