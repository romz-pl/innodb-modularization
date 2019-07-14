#pragma once

#include <innodb/univ/univ.h>

/** Error codes from os_file_get_last_error @{ */
constexpr ulint OS_FILE_NOT_FOUND = 71;
constexpr ulint OS_FILE_DISK_FULL = 72;
constexpr ulint OS_FILE_ALREADY_EXISTS = 73;
constexpr ulint OS_FILE_PATH_ERROR = 74;

/** wait for OS aio resources to become available again */
constexpr ulint OS_FILE_AIO_RESOURCES_RESERVED = 75;

constexpr ulint OS_FILE_SHARING_VIOLATION = 76;
constexpr ulint OS_FILE_ERROR_NOT_SPECIFIED = 77;
constexpr ulint OS_FILE_INSUFFICIENT_RESOURCE = 78;
constexpr ulint OS_FILE_AIO_INTERRUPTED = 79;
constexpr ulint OS_FILE_OPERATION_ABORTED = 80;
constexpr ulint OS_FILE_ACCESS_VIOLATION = 81;
constexpr ulint OS_FILE_NAME_TOO_LONG = 82;
constexpr ulint OS_FILE_ERROR_MAX = 100;
/* @} */
