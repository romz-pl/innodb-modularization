#include <innodb/tablespace/fil_delete_file.h>

#include <innodb/allocator/ut_free.h>
#include <innodb/io/Fil_path.h>
#include <innodb/io/os_file_delete_if_exists.h>
#include <innodb/io/pfs.h>

/** Delete the tablespace file and any related files like .cfg.
This should not be called for temporary tables.
@param[in]	path		File path of the IBD tablespace
@return true on success */
bool fil_delete_file(const char *path) {
  bool success = true;

  /* Force a delete of any stale .ibd files that are lying around. */
  success = os_file_delete_if_exists(innodb_data_file_key, path, nullptr);

  char *cfg_filepath = Fil_path::make_cfg(path);

  if (cfg_filepath != nullptr) {
    os_file_delete_if_exists(innodb_data_file_key, cfg_filepath, nullptr);

    ut_free(cfg_filepath);
  }

  char *cfp_filepath = Fil_path::make_cfp(path);

  if (cfp_filepath != nullptr) {
    os_file_delete_if_exists(innodb_data_file_key, cfp_filepath, nullptr);

    ut_free(cfp_filepath);
  }

  return (success);
}
