#include <innodb/log_types/log_test.h>

/** Represents currently running test of redo log, nullptr otherwise. */
std::unique_ptr<Log_test> log_test;
