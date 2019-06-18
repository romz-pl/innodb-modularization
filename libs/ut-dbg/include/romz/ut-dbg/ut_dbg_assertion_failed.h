#pragma once

#include <cstdint>

/** Report a failed assertion. */
[[noreturn]] void ut_dbg_assertion_failed(
    const char *expr, /*!< in: the failed assertion */
    const char *file, /*!< in: source file containing the assertion */
    uint64_t line);      /*!< in: line number of the assertion */
