#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/IORequest.h>

#define IORequestLogRead IORequest(IORequest::LOG | IORequest::READ)
