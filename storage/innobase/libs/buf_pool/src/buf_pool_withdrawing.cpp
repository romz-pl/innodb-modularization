#include <innodb/buf_pool/buf_pool_withdrawing.h>

/** true when withdrawing buffer pool pages might cause page relocation */
volatile bool buf_pool_withdrawing;
