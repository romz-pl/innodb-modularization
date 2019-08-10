#pragma once

#include <innodb/univ/univ.h>

int add_page_callback(void *context, byte *buff, uint num_pages);
