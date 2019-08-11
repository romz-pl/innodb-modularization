#include <innodb/buf_pool/buf_pool_should_madvise.h>

/* We maintain our private view of innobase_should_madvise_buf_pool() which we
initialize at the beginning of buf_pool_init() and then update when the
@@global.innodb_buffer_pool_in_core_file changes.
Changes to buf_pool_should_madvise are protected by holding chunk_mutex for all
buf_pool_t instances.
This way, even if @@global.innodb_buffer_pool_in_core_file changes during
execution of buf_pool_init() (unlikely) or during buf_pool_resize(), we will use
a single consistent value for all (de)allocated chunks.
The function buf_pool_update_madvise() handles updating buf_pool_should_madvise
in reaction to changes to @@global.innodb_buffer_pool_in_core_file and makes
sure before releasing chunk_mutex-es that all chunks are properly madvised
according to new value.
It is important that initial value of this variable is `false` and not `true`,
as on some platforms which do not support madvise() or MADV_DONT_DUMP we need to
avoid taking any actions which might trigger a warning or disabling @@core_file.
*/
bool buf_pool_should_madvise = false;
