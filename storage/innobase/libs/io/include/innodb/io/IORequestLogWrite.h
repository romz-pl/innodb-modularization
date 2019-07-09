#pragma once

#include <innodb/univ/univ.h>

#include <innodb/io/IORequest.h>

#define IORequestLogWrite IORequest(IORequest::LOG | IORequest::WRITE)
