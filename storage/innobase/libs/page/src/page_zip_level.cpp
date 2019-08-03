#include <innodb/page/page_zip_level.h>

#include <innodb/page/flag.h>

/* Compression level to be used by zlib. Settable by user. */
uint page_zip_level = DEFAULT_COMPRESSION_LEVEL;
