#pragma once

#include <innodb/univ/univ.h>

#include <innodb/error/dberr_t.h>

#include <string>

dberr_t fil_scan_for_tablespaces(const std::string &directories);
