#pragma once

#include <innodb/univ/univ.h>

/* Whether or not to log compressed page images to avoid possible
compression algorithm changes in zlib. */
extern bool page_zip_log_pages;
