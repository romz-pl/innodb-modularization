#include <innodb/page/page_zip_reorganize.h>

#include <innodb/dict_mem/dict_index_t.h>
#include <innodb/mtr/mtr_t.h>
#include <innodb/buf_block/buf_block_t.h>
#include <innodb/disk/page_t.h>
#include <innodb/machine/data.h>
#include <innodb/memory_check/memory_check.h>
#include <innodb/buf_pool/buf_pool_t.h>
#include <innodb/buf_pool/buf_pool_from_block.h>

