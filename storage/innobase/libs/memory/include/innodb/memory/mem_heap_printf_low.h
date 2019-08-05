#pragma once

#include <innodb/univ/univ.h>

#include <cstdarg>

ulint mem_heap_printf_low(
    char *buf,          /*!< in/out: buffer to store formatted string
                        in, or NULL to just calculate length */
    const char *format, /*!< in: format string */
    va_list ap);         /*!< in: arguments */
