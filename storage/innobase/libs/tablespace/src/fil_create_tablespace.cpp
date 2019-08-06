#include <innodb/tablespace/fil_create_tablespace.h>

#include <innodb/io/os_file_create_subdirs_if_needed.h>
#include <innodb/io/pfs_os_file_t.h>
#include <innodb/io/os_file_create.h>
#include <innodb/io/os_file_get_last_error.h>
#include <innodb/ioasync/os_file_set_size.h>
#include <innodb/tablespace/fil_space_t.h>
#include <innodb/tablespace/fsp_is_system_tablespace.h>
#include <innodb/tablespace/fsp_is_global_temporary.h>
#include <innodb/tablespace/fsp_flags_is_valid.h>
#include <innodb/tablespace/fil_fusionio_enable_atomic_write.h>
#include <innodb/disk/page_size_t.h>
#include <innodb/io/pfs.h>
#include <innodb/io/srv_read_only_mode.h>
#include <innodb/io/os_file_close.h>
#include <innodb/io/os_file_delete.h>
#include <innodb/io/os_is_sparse_file_supported.h>
#include <innodb/io/os_file_punch_hole.h>
#include <innodb/allocator/ut_malloc_nokey.h>
#include <innodb/align/ut_align.h>
#include <innodb/tablespace/fsp_flags_set_page_size.h>
#include <innodb/tablespace/fsp_header_init_fields.h>
#include <innodb/machine/data.h>
#include <innodb/tablespace/consts.h>
#include <innodb/io/IORequest.h>
#include <innodb/tablespace/fsp_is_checksum_disabled.h>
#include <innodb/page/page_zip_des_t.h>
#include <innodb/io/os_file_write.h>
#include <innodb/page/page_zip_set_size.h>
#include <innodb/allocator/ut_free.h>
#include <innodb/io/os_file_flush.h>
#include <innodb/tablespace/fil_space_create.h>
#include <innodb/tablespace/fil_system.h>


#include <innodb/log_types/lsn_t.h>

struct buf_block_t;
void buf_flush_init_for_writing(const buf_block_t *block, byte *page,
                                void *page_zip_, lsn_t newest_lsn,
                                bool skip_checksum, bool skip_lsn_check);


