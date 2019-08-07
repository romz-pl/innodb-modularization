#pragma once

#include <innodb/univ/univ.h>

#ifndef UNIV_HOTBACKUP

#include <innodb/log_redo/Log_test.h>

#include <memory>

/** Represents currently running test of redo log, nullptr otherwise. */
extern std::unique_ptr<Log_test> log_test;

#endif /* !UNIV_HOTBACKUP */
