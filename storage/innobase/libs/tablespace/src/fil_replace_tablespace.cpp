#include <innodb/tablespace/fil_replace_tablespace.h>

#include <innodb/io/create_type.h>
#include <innodb/io/os_file_close.h>
#include <innodb/io/os_file_create.h>
#include <innodb/io/os_file_create_t.h>
#include <innodb/io/pfs.h>
#include <innodb/io/srv_read_only_mode.h>
#include <innodb/ioasync/os_file_set_size.h>
#include <innodb/logger/error.h>
#include <innodb/logger/info.h>
#include <innodb/disk/univ_page_size.h>
#include <innodb/tablespace/consts.h>
#include <innodb/tablespace/fil_delete_tablespace.h>
#include <innodb/tablespace/fil_fusionio_enable_atomic_write.h>
#include <innodb/tablespace/fil_node_create.h>
#include <innodb/tablespace/fil_space_create.h>
#include <innodb/tablespace/fil_space_free.h>
#include <innodb/tablespace/fil_space_get.h>
#include <innodb/tablespace/fil_space_t.h>
#include <innodb/tablespace/fsp_flags_init.h>


extern ibool srv_use_doublewrite_buf;

/** Truncate the tablespace to needed size with a new space_id.
@param[in]  old_space_id   Tablespace ID to truncate
@param[in]  new_space_id   Tablespace ID to for the new file
@param[in]  size_in_pages  Truncate size.
@return true if truncate was successful. */
bool fil_replace_tablespace(space_id_t old_space_id, space_id_t new_space_id,
                            page_no_t size_in_pages) {
  fil_space_t *space = fil_space_get(old_space_id);
  std::string space_name(space->name);
  std::string file_name(space->files.front().name);
  bool is_encrypted = FSP_FLAGS_GET_ENCRYPTION(space->flags);

  /* Delete the old file and space object. */
  dberr_t err = fil_delete_tablespace(old_space_id, BUF_REMOVE_ALL_NO_WRITE);
  if (err != DB_SUCCESS) {
    return (false);
  }

  /* Create the new one. */
  bool success;
  pfs_os_file_t fh = os_file_create(
      innodb_data_file_key, file_name.c_str(),
      srv_read_only_mode ? OS_FILE_OPEN : OS_FILE_CREATE, OS_FILE_NORMAL,
      OS_DATA_FILE, srv_read_only_mode, &success);
  if (!success) {
    ib::error(ER_IB_MSG_1214, space_name.c_str(), "during truncate");
    return (success);
  }

  /* Now write it full of zeros */
  success = os_file_set_size(file_name.c_str(), fh, 0,
                             size_in_pages << UNIV_PAGE_SIZE_SHIFT,
                             srv_read_only_mode, true);
  if (!success) {
    ib::info(ER_IB_MSG_1074, file_name.c_str());
    return (success);
  }

  os_file_close(fh);

  uint32_t flags =
      fsp_flags_init(univ_page_size, false, false, false, false, is_encrypted);

  /* Delete the fil_space_t object for the new_space_id if it exists. */
  if (fil_space_get(new_space_id) != nullptr) {
    fil_space_free(new_space_id, false);
  }

  space = fil_space_create(space_name.c_str(), new_space_id, flags,
                           FIL_TYPE_TABLESPACE);
  if (space == nullptr) {
    ib::error(ER_IB_MSG_1082, space_name.c_str());
    return (false);
  }

  page_no_t n_pages = SRV_UNDO_TABLESPACE_SIZE_IN_PAGES;
#if !defined(NO_FALLOCATE) && defined(UNIV_LINUX)
  bool atomic_write = false;
  if (!srv_use_doublewrite_buf) {
    atomic_write = fil_fusionio_enable_atomic_write(fh);
  }
#else
  bool atomic_write = false;
#endif /* !NO_FALLOCATE && UNIV_LINUX */

  char *fn =
      fil_node_create(file_name.c_str(), n_pages, space, false, atomic_write);
  if (fn == nullptr) {
    ib::error(ER_IB_MSG_1082, space_name.c_str());
    return (false);
  }

  return (true);
}
