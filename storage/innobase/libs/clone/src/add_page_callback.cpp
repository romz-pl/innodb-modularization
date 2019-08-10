#include <innodb/clone/add_page_callback.h>

#include <innodb/clone/Clone_Snapshot.h>
#include <innodb/machine/data.h>

/** Callback to add tracked page IDs to current snapshot
@param[in]	context		snapshot
@param[in]	buff		buffer having page IDs
@param[in]	num_pages	number of tracked pages
@return	error code */
int add_page_callback(void *context, byte *buff, uint num_pages) {
  uint index;
  Clone_Snapshot *snapshot;

  ib_uint32_t space_id;
  ib_uint32_t page_num;

  snapshot = static_cast<Clone_Snapshot *>(context);

  /* Extract the page Ids from the buffer. */
  for (index = 0; index < num_pages; index++) {
    space_id = mach_read_from_4(buff);
    buff += 4;

    page_num = mach_read_from_4(buff);
    buff += 4;

    auto err = snapshot->add_page(space_id, page_num);

    if (err != 0) {
      return (err);
    }
  }

  return (0);
}

