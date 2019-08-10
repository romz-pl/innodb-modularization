#include <innodb/clone/add_redo_file_callback.h>

#include <innodb/clone/Clone_Snapshot.h>

/** Callback to add an archived redo file to current snapshot
@param[in]	file_name	file name
@param[in]	file_size	file size in bytes
@param[in]	file_offset	start offset in bytes
@param[in]	context		snapshot
@return	error code */
int add_redo_file_callback(char *file_name, ib_uint64_t file_size,
                                  ib_uint64_t file_offset, void *context) {
  auto snapshot = static_cast<Clone_Snapshot *>(context);

  auto err = snapshot->add_redo_file(file_name, file_size, file_offset);

  return (err);
}
