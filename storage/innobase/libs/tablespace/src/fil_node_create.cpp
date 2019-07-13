#include <innodb/tablespace/fil_node_create.h>

#include <innodb/tablespace/fil_system.h>
#include <innodb/io/IORequest.h>

/** Attach a file to a tablespace. File must be closed.
@param[in]	name		file name (file must be closed)
@param[in]	size		file size in database blocks, rounded
                                downwards to an integer
@param[in,out]	space		space where to append
@param[in]	is_raw		true if a raw device or a raw disk partition
@param[in]	atomic_write	true if the file has atomic write enabled
@param[in]	max_pages	maximum number of pages in file
@return pointer to the file name
@retval nullptr if error */
char *fil_node_create(const char *name, page_no_t size, fil_space_t *space,
                      bool is_raw, bool atomic_write, page_no_t max_pages) {
  auto shard = fil_system->shard_by_id(space->id);

  fil_node_t *file;

  file = shard->create_node(name, size, space, is_raw,
                            IORequest::is_punch_hole_supported(), atomic_write,
                            max_pages);

  return (file == nullptr ? nullptr : file->name);
}
