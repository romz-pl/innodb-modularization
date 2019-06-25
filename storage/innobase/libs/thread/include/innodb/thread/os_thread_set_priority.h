#pragma once

#include <innodb/univ/univ.h>

bool os_thread_set_priority(int priority);

void os_thread_set_priority(int priority, const char *thread_name);
