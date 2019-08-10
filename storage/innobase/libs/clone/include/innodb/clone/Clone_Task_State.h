#pragma once

#include <innodb/univ/univ.h>


/** Clone task state */
enum Clone_Task_State {
    CLONE_TASK_INACTIVE = 1,
    CLONE_TASK_ACTIVE
};
