#include <innodb/log_sys/log_sys_object.h>

/** Redo log system. Singleton used to populate global pointer. */
aligned_pointer<log_t> *log_sys_object;
